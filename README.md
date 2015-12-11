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
    --data          data file path (for cocos2d it will be .plist)                          [default: same path with result texture]
    --scale         scale image factor (at 0 to 1)                                          [default: "1"]
    --trim          trims source images according to the mode (none, all-alpha, max-alpha)  [default: "max-alpha"]
    --padding       general padding between sprites and border                              [default: "0"]
    --inner-padding distance between sprites                                                [default: "1"]
    --suffix        path extension which will be used by the atlas data file                [default: same as resulting texture]
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

###Output###
SpriteGlue generates texture in png format. Next you may want to turn this png texture into a platform depended format like PVR, PKM etc.
For this purpose use following options:

1. **--suffix**
    Gives you posibility to pre-define the final texture file extension in your atlas meta data file (for cocos2d it's plist)

2. **--square**
    Some formats like PVR supports only squared images, that why you must use this option if you going to convert a texture into PVR

3. **--powerOf2**
    Some render systems like OpenGL ES 1.1 and graphic formats like PVR support textures where width and height aliquot to power of 2

###Algorithm/Performance###
As a layout algorithm has been selected "max-rects" and using some additional aligning/sprite duplicating features makes your atlas as compact as posible, I hope:)
The atlases made by SpriteGlue have been even a little bit compact then same atlases made by commercial TexturePacker.
  
###Trimming / Cropping###
SpriteGlue can remove transparent whitespace around images. With that you can pack more assets into one spritesheet and it makes rendering a little bit faster.
