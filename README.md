![spritesheet.js](http://i.imgur.com/RcHZ2qZ.png)
==============

Spritesheet.js is command-line spritesheet (a.k.a. Texture Atlas) generator written in node.js.

###Supported spritesheet formats###
* Starling / Sparrow
* JSON (i.e. PIXI.js)
* Easel.js
* cocos2d
* CSS (new!)

###Usage###
1. **Command Line**
    ```bash
    $ spritesheet-js assets/*.png
    ```
    Options:
    ```bash
    $ spritesheet-js
    Usage: spritesheet-js [options] <files>
	
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
2. **Node.js**
    ```javascript
    var spritesheet = require('spritesheet-js');
    
    spritesheet('assets/*.png', {format: 'json'}, function (err) {
      if (err) throw err;

      console.log('spritesheet successfully generated');
    });
  ```
  
###Trimming / Cropping###
Spritesheet.js can remove transparent whitespace around images. Thanks to that you can pack more assets into one spritesheet and it makes rendering a little bit faster.

*NOTE: Some libraries such as Easel.js dont't support this feature.*
![Trimming / Cropping](http://i.imgur.com/76OokJU.png)
