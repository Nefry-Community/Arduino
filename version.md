1.2.0 追加機能

- Captive Portal
- DNS
- Nefry_Milkcocoaライブラリー
- Nefry変数名変更
- Nefryサンプルプログラム変更

1.2.1 

- Macにてコンパイルエラーが起こったため修正

1.2.2

- ajaxでコンソールページを自動更新

1.3.0 beta1

- SPIFFSにてHTMLファイル保存
- Nefryライブラリ関数一部変更（プライベート、返り値の型）
- SPIFFSで書き込むプログラムを追加

1.4.0

- ボードの追加（nefry v2,cocoabit）
- 関数の追加（getProgramName,setProgramName,autoConnect）
- HTMLファイルの変更

1.4.1

- CococaBitのピン変更
- Serialのprint関数を変更
- WiFiクラッシュを修正[qiitaを参考](http://qiita.com/7of9/items/73f9d216f9810333ef9b)

1.4.2

- Libraryのバージョンの変更
- 書き込みモードかどうかのWeb表示

2.0.0(1.5.0)

- Arduino core for ESP8266 WiFi chip のstableVersion2.3.0への変更

2.0.1

- 不要なファイルの削除（ボード関係）
- getModuleName関数の追加

2.0.2

- memoryの使用量の削減(10kb)

2.1.0

- autoUpdate関数の追加
- WriteModeライブラリーの追加
- CocoaBitのライブラリーを追加

2.1.1

- 16進数でのLED制御関数の追加
- サンプルプログラムの修正
- WebConsoleの送信をAjax対応

2.2.0

- setIndexLink 関数の追加
- getWebServer 関数の追加
- Nefry_Milkcocoaライブラリの修正
- CocoaBitライブラリの更新
- NefryRESTAPIライブラリの追加

2.2.1

- EasyEditor追加
- EspExceptionDecoderに対応
- CocoaBit,RESTAPI更新
- プログラムを更新する際に自動更新停止モードを追加

2.2.2

- RESTAPIのバージョンアップに関する修正
- アップデート関連の調整
  - NefryについているLEDがアップデートに成功した際に青点灯、失敗した場合に赤点灯するように変更
  - 自動更新が止まったときの注意文の変更
- RestAPIに関するCocoaBitのI2Cなどのピン情報の有効化

2.2.3

- setConfHtmlStr＆setConfHtmlValue関数の追加
- WriteModeを赤色に変更
- CaptivePortal向けにFormを修正
- Webからのダウンロードを簡単にするためにドメインを入力
- easyEditor.NefryRESTAPI.CocoaBitライブラリを更新

2.3.0

- 5つまでのWiFiを登録し、その中から最も強いWiFiに接続する機能を追加
- WiFiが切断した場合、再接続する機能をデフォルトで追加（今までは関数としてあった）
- Nefry Web Configの全体的な改善

関数の追加

- readSW
- setWifiTimeout
- getWifiTimeout
- setWiifAuto
- getWifiAuto
- addWifi
- deleteWifi
- searchWiFi
- getlistWifi
- autoUpdate　引数の変更
