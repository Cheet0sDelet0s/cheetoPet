<h2 align="center"><u>cheetoPet</u></h2>

![**an esp32 s3 based virtual pet**](/photos/main.jpg)
<h4 align="center"> an esp32 s3 based virtual pet </h4>

<p align="center">
    <img src="https://img.shields.io/github/stars/Cheet0sDelet0s/cheetoPet?style=for-the-badge&color=orange">
    <img src="https://img.shields.io/github/issues/Cheet0sDelet0s/cheetoPet?style=for-the-badge&color=red">
<br>
    <img src="https://img.shields.io/badge/Open%20Source-Yes-orange?style=flat-square">
    <img src="https://img.shields.io/badge/Written%20In-C++-blue?style=flat-square">
</p>

### [+] notice
cheetoPet is being completely rebuilt. the pcb and software is still not finished. please star this project and come back to it later!


### [+] description
cheetoPet is a fully open-source modern tamagotchi. it features a 240*280 colour display, 3 face buttons + side rocker, 6 axis gyro and a speaker, all built on a custom pcb.




### [+] software
the software uses an abstraction layer that allows it to be ran on both esp32 and PC, where drivers for different devices can be easily created and swapped in. for PC, a lot of hardware that is present on the cheetoPet board is simulated, such as the gyro and battery. this is so the program runs as close to how it would natively. this is mostly to make development far easier, as constantly having to build the program and wait for it to upload to your esp32 takes a significant amount of time. being able to build and run it locally, while having access to regular c++ debugging tools, makes everything much less annoying.


### [+] building for pc
use the build.sh bash script in cheetoPet-V2:
`./build.sh`
you will need gcc and SDL2 installed. if the build is successful, the executable will run automatically. i develop on linux, so you may have issues if you are on windows/macos. if you are - why? without sounding like an annoying arch user (which i am), linux is simply better.

### [+] uploading to esp32
arduino IDE/CLI or the arduino vscode extension (what i use) is required to build and upload to the board. install the esp32 core in board manager, select esp32s3 dev module as your device, enable USB CDC on boot in tools and upload.

### [+] ordering pcb
i use JLCPCB for all my pcbs. they are simply the cheapest option available, but you can use any service really. just upload the gerbers and probably order a stencil to go with it. no specific board settings are needed really, though you may want to get a lower board thickness.

### [+] ordering parts
LCSC (sister company of jlcpcb)  is where i get my parts for this boardd. aliexpress unfortunately doesnt really work here. its a sad day for us all. there is a BOM you can upload however there are most likely a few parts on it that are out of stock. finding alternatives is fairly easy.

you must buy the battery and display separately. a 1S 1000mah lipo/ion is what the board expects. the display is a very specific ST7789 from BuyDisplay, you can find it on aliexpress thank GOD. https://www.aliexpress.com/item/1005008850313883.html
they are about $5-6 each.

### [+] ordering pcba
i probably wouldnt even bother assembling by hand like i did, however getting it assembled for you is just so expensive for low quantity orders. jlcpcb quoted me £150 ($200, not including battery and display cost) for 2 boards, when building 4 by hand costs £100. so unless you are ordering 1000 units, you are kind of screwed either way 🥰🥰🥰

