#define TNZU_DEFINE_INTERFACE
#include <algorithm>
#include <toonz_utility.hpp>

class MyFx : public tnzu::Fx {
  public:
  //
  // PORT
  //
  enum {
    PORT_INPUT,
    PORT_COUNT,
  };
  int port_count() const override { return PORT_COUNT; }
  char const *port_name(int i) const override {
    static std::array<char const *, PORT_COUNT> names = {
        "Input",
    };
    return names[i];
  }

  //
  // PARAM GROUP
  //
  enum {
    PARAM_GROUP_DEFAULT,
    PARAM_GROUP_COUNT,
  };
  int param_group_count() const override { return PARAM_GROUP_COUNT; }
  char const *param_group_name(int i) const override {
    static std::array<char const *, PARAM_GROUP_COUNT> names = {
        "Default",
    };
    return names[i];
  }

  //
  // PARAM
  //
  enum {
    PARAM_SRC_TEMP,
    PARAM_DST_TEMP,
    PARAM_COUNT,
  };
  int param_count() const override { return PARAM_COUNT; }
  ParamPrototype const *param_prototype(int i) const override {
    static std::array<ParamPrototype, PARAM_COUNT> const params = {
    // name, group, default, min, max
      ParamPrototype{"From", PARAM_GROUP_DEFAULT, 6500, 2000, 11999},
      ParamPrototype{"To",   PARAM_GROUP_DEFAULT, 6500, 2000, 11999},
    };
    return &params[i];
  }

  public:
  
  template <typename Vec4T>
  int kernel(Config const &config, Params const &params, Args const &args, cv::Mat &retimg);

  int compute(Config const &config, Params const &params, Args const &args, cv::Mat &retimg) override try {
    DEBUG_PRINT(__FUNCTION__);
    if (args.invalid(PORT_INPUT)) {
      return 0;
    }
    if (retimg.type() == CV_8UC4) {
      return kernel<cv::Vec4b>(config, params, args, retimg);
    } else {
      return kernel<cv::Vec4w>(config, params, args, retimg);
    }
  } catch (cv::Exception const &e) {
    DEBUG_PRINT(e.what());
    return 0;
  }
};

// 色温度からリニアsRGBに変換する(2000K-12000K)
void temperatureToBgr(double _temperature, double _bgr[3]) {
  const double bgr_curve_coefs[3][6] = {
    {
      86.46296631772294, -177008.18272048747,   21465836.385942165,
      38.20840934285013,   51393.539358847614, 493414332.70288277
    }, {
      1.8380496785560545, 2821.609457661375,  -3661191.7606562786,
      1.9172701330323763, 1406.2126623674094,  3637103.982055007
    }, {
      271.36121481147853,  959362.0349363107, 7179890566.048965,
      394.85272594721937, 1043949.4526673924,  448306406.4642674
    }
  };
  double temperature = std::clamp(_temperature, 2000.0, 12000.0);
  for (int i = 0; i < 3; i++) {
    double num = bgr_curve_coefs[i][0];
    for (int j = 1; j < 3; j++) {
      num = num * temperature + bgr_curve_coefs[i][j];
    }
    double den = bgr_curve_coefs[i][3];
    for (int j = 1; j < 3; j++) {
      den = den * temperature + bgr_curve_coefs[i][j+3];
    }
    _bgr[i] = num / den;
  }
}

// srcの色温度をdstの色温度にするために掛けるBGRの係数を計算する
void calcBgrCoef(double const _srcTemp, double const _dstTemp, float _resultBgr[3]) {
  double srcBgr[3];
  double dstBgr[3];
  double resultBgr[3];
  temperatureToBgr(_srcTemp, srcBgr);
  temperatureToBgr(_dstTemp, dstBgr);
  for (int i = 0; i < 3; i++) {
    resultBgr[i] = dstBgr[i] / srcBgr[i];
  }
  double maxCoef = std::max(std::max(resultBgr[0], resultBgr[1]), resultBgr[2]);
  for (int i = 0; i < 3; i++) {
    _resultBgr[i] = static_cast<float>(resultBgr[i] / maxCoef);
  }
}

template <typename Vec4T>
int MyFx::kernel(Config const &config, Params const &params, Args const &args, cv::Mat &retimg) {
  using value_type = typename Vec4T::value_type;

  float const gamma = 2.2f;
  double const srcTemp = params.get<double>(PARAM_SRC_TEMP);
  double const dstTemp = params.get<double>(PARAM_DST_TEMP);

  args.get(PORT_INPUT).copyTo(retimg(args.rect(PORT_INPUT)));

  tnzu::linear_color_space_converter<sizeof(value_type) * 8> converter(1.0f, gamma);

  float bgr_coef[3];
  calcBgrCoef(srcTemp, dstTemp, bgr_coef);

  #ifdef _OPENMP
  #pragma omp parallel for
  #endif
  for (int y = 0; y < retimg.rows; y++) {
    Vec4T* scanline = retimg.ptr<Vec4T>(y);
    for (int x = 0; x < retimg.cols; x++) {
      for (int c = 0; c < 3; c++) {
        float ch_value;
        ch_value = converter[scanline[x][c]];
        ch_value = ch_value * bgr_coef[c];
        ch_value = tnzu::to_nonlinear_color_space(ch_value, 1.0f, gamma);
        scanline[x][c] = tnzu::normalize_cast<value_type>(ch_value);
      }
    }
  }

  return 0;
}

namespace tnzu {
  PluginInfo const *plugin_info() {
    static PluginInfo const info(
      TNZU_PP_STR(PLUGIN_NAME),   // name
      TNZU_PP_STR(PLUGIN_VENDOR), // vendor
      "",                         // note
      "https://github.com/hokudaianime/OpenToonz_Plugin_ColorTemp"  // helpurl
    );
    return &info;
  }

  Fx *make_fx() { return new MyFx(); }
}
