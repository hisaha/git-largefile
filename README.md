# what is git-big


オリジナルは git-largefileです。

  * http://dsas.blog.klab.org/archives/52105107.html
  * https://github.com/methane/git-largefile

開発のコンセプトとしてはgit-mediaとほぼ同じようですが

  1. gitは大きいバイナリを扱うとレポジトリサイズが膨らむ
  2. git annex等の解決策があるが、けっこう導入コスト高い
  3. gitのフィルタでなんとかならんか

ということのようです。

開発チームがsourcetreeで回っているとかだと、git annexのコマンドとか覚えるはけっこうしんどいときもあるかなと思いますので、よいアイデアだと思います。

git-mediaがrsyncをサポートしていなかったので、git-largefileのオリジナルはS3へ保存する形へ舵を切られたようで、S3だとコストが気になるケースもありますので、rsync型のままで自分で使い易いようにFitさせる目的でfork/branchしてます。

  * 拠点(社内)にRAID組んだrsyncサーバーがあって
  * 拠点外からもVPNとかでrsyncサーバーにはアクセスできて

という環境で楽に動作するように考えました。

まだプロトタイプなのでそのうちハッシュ高速化のためにCで書き直すと思います。

git annexがsorucetreeにサポートされたり、git mediaがrsync対応してくれれば自分の環境ではそれでも全く問題ないのですが、一応自分の勉強のログ残しということで。

## 動作

gitbigフィルタはgitからみると

* ファイルの中身を渡したらハッシュ値を出力するフィルタ
* ハッシュ値を渡したらファイルの中身を出力するフィルタ

として機能します。gitはgitbigフィルタから出力されたハッシュ値のみのファイルをバージョン管理していくことになります。

* git addのときに、gitbigフィルタの機能で大きなファイルはハッシュ値のみのファイルになる
* gitはそのファイルのハッシュ値を計算し、バージョン管理する
* git checkoutなんかのときには、ハッシュ値のみのファイルをcheckoutした後にgitbigフィルタに通して、ファイルの中身を得る

という動作になります。

gitbigフィルタは、内部的に

* ファイルの中身を受取ったらハッシュ値にしてしまい、適当なサーバーにrsyncする
* ハッシュ値を受け取ったら適当なサーバーからローカルのディレクトリにrsyncし、ハッシュ値に対応するファイルを出力する

という動作をします。

gitのフィルタやrsyncサーバーの設定等が終わっていれば、後はあまり意識せずにsourcetreeなんかのUIアプリででも普通に動作します。

git annexなんかがお好みのUIツールにサポートされれば、そちらのほうがよいかも知れませんね。

## 設定方法

### インストール

* gitbig.pyを/usr/local/bin/とかにコピー
* rsync,opensslをインストール

以上

### gitconfig

`.git/config` に、次のように設定してください

```
[filter "gitbig"]
    clean = /usr/local/gitbig.py <opt>
    smudge = /usr/local/gitbig.py <opt>
```

これで gitbig フィルターが動きます.

### gitattribute

git リポジトリの中に `.gitattributes` っていうファイルを作って、次のように設定してください。

```
*.wav  filter=gitbig
*.aif filter=gitbig
*.aiff  filter=gitbig
*.aac  filter=gitbig
*.aac  filter=gitbig
*.aac  filter=gitbig
*.aac  filter=gitbig
```

これで設定したファイルは gitbig フィルターを通るようになります.

書きかけですがとりあえず。