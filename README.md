# Cloak
Hide and extract an encrypted file within an RGB (24-bit) bitmap or PNG image. 

The idea is simple, a 24-bit colour bitmap or PNG image uses 3 bytes for each pixel in the image, one each for Red, Green and Blue, so each colour channel is represented by a value between 0 - 255. If we encode data in the least significant bits (LSBs) of the image data, there will be no visible difference in the image when displayed. At an encoding depth of 1-bit per byte, we need 8 bytes of image data to encode 1 byte of our file.

Cloak will optionally encrypt your 'secret' file using 2 seperate passes through the 128-bit AES cipher prior to encoding it in your chosen image. You will be prompted to enter a password which is used as the key for the first pass through AES, a seperate derived key is then used for the second pass. Cloak can also encrypt using a supplied keystream file using simple XOR encryption, the advantage of this mechanism is you can employ a one-time-pad scheme, which providing you stick to the rules for a one-time-pad encryption scheme, is mathematically proven to be unbreakable. Of course, any encryption scheme is useless if some third party has got hold of your encryption key.

Building Cloak
--------------
Cloak is written in C++ and I have provided makefiles for Unix/Linux (makefile.nix) using the g++ (gcc) compiler (tested on Ubuntu 16.04) and for Windows (makefile.vcwin32) using Visual Studio C++ (I used VS 2015 Community Edition, tested on Windows 10). Cloak depends on the 3rd party libraries libpng, zlib (both for PNG read/write functionality) and libbsd for Linux (to provide strlcpy/strlcat library functions). I have provided the pre-built libraries libpng.lib and zlib.lib for Windows x64.

Using Cloak
-----------
Cloak has two modes of operation, command line (driven by command line parameters) and interactive mode (you enter commands at the cloak> prompt). Type Cloak -? to get help on the command line parameters, if you just type Cloak [enter] you will enter interactive mode and can type the 'help' command for a list of available commands.

I have included a sample PNG file with this distribution - flowers.png which has a PDF document encoded within it, the password used to encrypt the file is 'password', I strongly suggest you use much stronger passwords :)

For example, to 'cloak' a file within flowers.png I used the following command:

    cloak -a -i in.png -s secret.pdf -o flowers.png -b1
    
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
    Enter output image filename: flowers.png
    Enter keystream filename (Enter = none): 
    Enter password: ********
    Confirm password: ********
    cloak> quit
    bye...

and to 'uncloak' the file:

    cloak> load image 
    Enter input image filename: flowers.png
    cloak> extract file
    Enter bits per byte (1, 2, or 4): 1
    Enter extracted filename: out.pdf
    Enter keystream filename (Enter = none): 
    Enter password: ********
    Confirm password: ********
    cloak> quit
    bye...

In the case of PNG files, you may notice that the output PNG image from Cloak is significantly smaller than the input image, they are identical in content, it is simply that I have configured libpng in Cloak to use the maximum compression ratio. This is the key difference between PNG images and 24-bit bitmap images, PNG is compressed whereas bitmap (bmp) is not. If you use a bitmap (bmp) file with Cloak you will see that the input and output .bmp files will be exactly the same size.

Have fun!
