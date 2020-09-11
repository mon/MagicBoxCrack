# Magic Box Tools

Tooling to decrypt and run the jubeat clone, Magic Box.

This is an absolutely classic Chinese knockoff, with terrible English, songs
ripped from random places, awful charting, broken timing and a horrendous UI.

As far as I can tell the company producing this cab went under in 2013.

![sample image](./sample.png)

## Quickstart

Download the latest [release](https://github.com/mon/MagicBoxCrack/releases).

Find a copy of Magic Box 4.9 and extract the files from the release into the
Magic Box folder. I'd host it myself but it contains a lot of copyrighted music.

### **EXTREMELY IMPORTANT**
DELETE `play.bat`. Running this file will DESTROY YOUR WINDOWS INSTALL.

This crack uses the jbio.dll interface from Bemanitools 5.

### I have a keyboard or something that pretends to be a keyboard
Find a copy of Bemanitools (I know you can do it!) and take jbio.dll and
geninput.dll and put it in the Magic Box folder. Configure inputs using
config.exe.

### I have a jubeat cab and I want to run using p4io
Rename `jbio-p4io.dll` to `jbio.dll`. I personally use `Magic_windowed.exe`,
fullscreen mode stretches kinda funny.

## I'm ready to go!
Run either `Magic.exe` or `Magic_windowed.exe`.

## I'm having problems
I don't want to support this game any further than what I have already done.
You can open an issue, but I probably won't fix it.

## What are the folders/files in this repo?
- `magic_decrypt.py` decrypts all the `.ltm` files into `.png` or `.txt`. It's
janky code. Edit the paths in the .py and run it.
- `magic_dumper` can be run on a genuine Magic Box cab to dump dongle keys
- `magic_iohook` is the hook code required to get the game booting. Use a
Windows XP toolchain for full cab compatibility.
- `magic_iohook/unicode` is ICU 58, the last version compatible with Windows XP.
This is used to convert the Chinese filenames in music.ini to OS independant
UTF16. Using this, the game happily runs on the WinXPE my jubeat cab came with.
