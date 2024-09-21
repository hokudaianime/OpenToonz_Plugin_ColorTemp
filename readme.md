# Color Temperature Plugin for OpenToonz

OpenToonzで色温度を調整できるプラグインです。

## インストール

使っているOSに対応した`.plugin`の拡張子がついているファイルを、
Windowsのデフォルトの場合は
`C:\OpenToonz stuff\plugins\`
に、
Macのデフォルトの場合は
`/Applications/OpenToonz/OpenToonz_stuff/plugins/`
に配置すれば読み込まれると思います。

## つかいかた

画像の中の元の色の色温度を`From`に、目標の色温度を`To`に設定すると、`From`の色温度の色が`To`の色温度の色になります。

## ビルド方法

少なくともOpenToonz v1.7.1の場合、OpenCV 4.5.1が必要です。

リポジトリをクローンしたディレクトリの中にbuildディレクトリを作り、その中で
- Windowsの場合
```Shell
cmake -DOpenCV_DIR="（OpenCVのOpenCVConfig.cmakeがある場所へのパス）" ..
cmake --build . --config Release
```
- Macの場合
```Shell
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 -DOpenCV_DIR="（OpenCVのOpenCVConfig.cmakeがある場所へのパス）" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
の2つのコマンドを打つとビルドされます。

プラグイン本体はWindowsの場合`bin/Release/`に、Macの場合`lib`に出力されます。
