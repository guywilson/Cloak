# Cloak
Hide and extract an encrypted file within an RGB (24-bit) bitmap or PNG image. 

The idea is simple, a 24-bit colour bitmap or PNG image uses 3 bytes for each pixel in the image, one each for Red, Green and Blue, so each colour channel is represented by a value between 0 - 255. If we encode a file in the least significant bits (LSBs) of the image data, there will be no visible difference in the image when displayed. At an encoding depth of 1-bit per byte, we need 8 bytes of image data to encode 1 byte of our file.

Cloak compresses and then optionally encrypts your 'secret' data file using the 128-bit AES cipher prior to encoding it in your chosen image. You will be prompted to enter a password (max 64 chars), the SHA-512 hash of which is used as the key for the pass through AES. Cloak can also encrypt using a supplied keystream file using simple XOR encryption, the advantage of this mechanism is you can employ a one-time-pad scheme, which providing you stick to the rules for a one-time-pad encryption scheme, is mathematically proven to be unbreakable. Of course, any encryption scheme is useless if some third party has got hold of your encryption key.

Some hints regarding password strength
--------------------------------------
A good password is one that cannot be broken using a dictionary attack, e.g. don't use a word from the dictionary or a derivation of. Use a made-up word or phrase with symbols and numbers, better stil a random string of characters. In the context of this software, an important aspect is getting the password to your intended audience securely.

References:

https://en.wikipedia.org/wiki/Dictionary_attack

https://en.wikipedia.org/wiki/Password_strength

Building Cloak
--------------
Cloak is written in C++ and I have provided makefiles for Unix/Linux (makefile.nix.mak) using the gcc compiler (tested on Ubuntu 16.04 64-bit), for Windows (makefile.win.mak) using Visual Studio C++ (I used VS 2015 Community Edition, tested on Windows 10 64-bit), and for macOS (makefile.mac.mak) using gcc (xcode commnad line tools) and tested on macOS Sierra 10.12.6. Cloak depends on the 3rd party libraries libpng, zlib (both for PNG read/write functionality), libsph - http://www.saphir2.com/sphlib/ (for SHA-512) and libbsd for Ubuntu (to provide strlcpy/strlcat library functions). I have provided the pre-built libraries libpng.lib and zlib.lib for Windows x64.

It is recommended that you copy the system specific makefile.xxx.mak to the file 'makefile', e.g. on macOs

    cp makefile.mac.mak makefile

Build cloak using the system make utility, e.g. on Linux/macOs

    make
    
and on Windows using Microsoft's make utility

    nmake

I've included an install target in the macOs and Linux makefiles which will copy the target executable to /usr/local/bin:

    make install
    
On Windows you can copy the executable (cloak.exe) to a location on the path, or add the directory where the executable has been built to the path:

    set PATH=PATH;C:\development\Cloak

Using Cloak
-----------
Cloak has two modes of operation, command line (driven by command line parameters) and interactive mode (you enter commands at the cloak> prompt). Type Cloak -? to get help on the command line parameters, if you just type Cloak [enter] you will enter interactive mode and can type the 'help' command for a list of available commands.

I have included a sample PNG file with this distribution - flowers.png which has a PDF document encoded within it, the password used to encrypt the file is 'password', I strongly suggest you use much stronger passwords :)

For example, to 'cloak' a file within flowers.png I used the following command:

    cloak -a -i in.png -s secret.pdf -o flowers.png -b1 -l5
    
This tells Cloak to use add (merge) mode to encode the file 'secret.pdf' into the image 'in.png' and output the new image 'flowers.png' using an encoding depth of 1-bit per byte.

To 'uncloak' the file from flowers.png, you can use the following command:

    cloak -e -i flowers.png -o out.pdf -b1
    
This tells Cloak to use extract mode to extract the file 'out.pdf' from the input image 'flowers.png', again using 1-bit per byte.

Using 'interactive' mode, a typical session to encode a file would look like this:

    cloak> load image
    Enter input image filename: in.png
    cloak> load file
    Enter input filename: secret.pdf
    cloak> merge
    Enter bits per byte (1, 2, or 4): 1
    Enter compression level (0 - 9, Enter = default): 5
    Enter output image filename: flowers.png
    Enter keystream filename (Enter = none): 
    Enter password: ********
    cloak> quit
    bye...

and to 'uncloak' the file:

    cloak> load image 
    Enter input image filename: flowers.png
    cloak> extract
    Enter bits per byte (1, 2, or 4): 1
    Enter extracted filename: out.pdf
    Enter keystream filename (Enter = none): 
    Enter password: ********
    cloak> quit
    bye...

Tips
----
* In the case of PNG files, you may notice that the output PNG image from Cloak is a different size than the input image, they are identical in content, it is simply that PNG compression will likely result in different size files. This is the key difference between PNG images and 24-bit bitmap images, PNG is compressed whereas bitmap (bmp) is generally not. If you use a bitmap (bmp) file with Cloak you will see that the input and output .bmp files will be exactly the same size.
* Try to use an image that is quite 'busy', the flowers.png supplied here is a good example. If you use an image with large areas of a single colour, it may be possible (but unlikely) to spot something encoded within the image, particularly if it is examined with a hex editor for example.

Have fun!

To Do
-----
* A drag and drop UI (yeah right)
