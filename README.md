spriteglue
==============

SpriteGlue is command-line spritesheet (a.k.a. Texture Atlas) generator written in Qt.
You can use it on any platform which supports Qt (Mac OS, Windows, Linux)

###Supported spritesheet formats###
* cocos2d

###Usage###
* **Command Line**
    ```bash
    $ spriteglue assets_folder
    ```
    Options:
    ```bash
    $ spriteglue
    Usage: spriteglue [options] <files> --sheet resultPath
	
	Options:
    --data          data file path (for cocos2d it will be .plist)                          [default: "same path with result texture"]
    --scale         scale image factor (at 0 to 1)                                          [default: "1"]
    --trim          trims source images according to the mode (none, all-alpha, max-alpha)  [default: "max-alpha"]
    --padding       general padding between sprites and border                              [default: "0"]
    --inner-padding distance between sprites                                                [default: "1"]
    --suffix        path extension which will be used by the atlas data file                [default: "same as resulting texture"]
    --max-size-w    max atlas width. if undefined it will use height instead                [default: "4096"]
    --max-size-h    max atlas height. if undefined it will use width instead                [default: "4096"]
    --square        makes texture width and height equal                                    [default: false]
    --powerOf2      makes texture size power of 2                                           [default: false]
    --opt           color format of resulting texture (rgb888, rgb666, rgb555, rgb444, alpha8, grayscale8, mono, rgba8888p) [default: "rgba8888"]
    ```

* **Example**
    ```bash
    spriteglue /Users/tovchenko/myassets --sheet /Users/tovchenko/myatlas.png --max-size-w 2048 --scale 0.5 --suffix pvr.ccz --square --powerOf2
    ```
  
###Trimming / Cropping###
SpriteGlue can remove transparent whitespace around images. Thanks to that you can pack more assets into one spritesheet and it makes rendering a little bit faster.

![Trimming / Cropping](http://i.imgur.com/76OokJU.png)
