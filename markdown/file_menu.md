% G-Pascal File menu

**Author**: Nick Gammon

**Date**: February 2022

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Text editor](editor.htm) </div>

The File menu, which was originally used in the Commodore 64 G-Pascal to load and save files, is used here to assist in loading and saving via the serial port. As described in the [editor](editor.htm) section, simply entering Insert mode and dumping a whole lot of text into the Editor will not work, because the Editor tries to tokenise each line, and will fall behind the rate of the incoming source. Instead use Load as described below, and Save to get a copy of the current source without line numbers. There is also an ability to Append more source to the existing source, and a Recover function, in case you press Reset and lose your source.

Enter the File menu by pressing **F** from the main menu, or **QF** from the Editor.

The File menu looks like this:

```
 <L>oad, <A>ppend, <S>ave, <E>dit, <I>nfo, <R>ecover, <Q>uit ?
```

---

## Load

This loads your source into memory basically copying it as quickly as possible into memory as it is received. If you have more than four lines of source already you will be asked to confirm that the existing source is to be overwritten. You may see, for example, a message like this:

```
 Do you want to delete 657 lines ? y/n
```

If you do not press **Y** then the load is aborted.

If you continue with the load you will see:

```
Paste source, terminate with Ctrl+D
```

Now is the time to copy the source from your Mac/PC, and then use the Paste function in your terminal program (or "send file" if there is such a function). Once the file has been sent press Ctrl+D to exit the loading mode.

*Do not type anything while the file is loading! That will corrupt it. Just wait until it has finished.*

Once you have finished loading you will see something like this:

```
 source ended at $3a63
Tokenising ...
 source ended at $2f04
 source CRC $efdf
 source length 14178

```

What this is telling you is that initially the source ended at address $3A63. Then it is tokenised which may take a few seconds, depending on how long it is. In the case of the Adventure game, on a processor running at 1 MHz, it takes six seconds. Then you get a second message about the new source ending address ($2F04 in this case). In this particular case tokenising has saved 2911 bytes ($3A63 - $2F04 = $B5F which is 2911 in decimal). Clearly tokenising is useful, to help reduce the amount of space taken by source where you have limited RAM.

You are then told the source length (14178 bytes) and Cyclic Redundancy Check (CRC) which is $EFDF. This can be used to verify that the source loaded without errors. In this case I am using "Jacksum" to do the CRC on my PC.

```
jacksum -a crc:16,1021,FFFF,false,false,0 -x adventure.pas

efdf	14178	adventure.pas
```

You can see that both the file length (14178) and the checksum ($efdf) agree, therefore the file loaded reliably.

You can re-check the CRC later by using the **I** (Info) option in the file menu. Of course, the CRC will change as you change the source.

---

## Append

Append works similarly to Load, however the incoming source is appended to your existing file rather than replacing it.

---

## Save

Save works similarly to List in the Editor, however line numbers are not shown at the start of each line. The intention here is to use Save to simply make a copy of your source, which you can then select in your terminal program, copy, and paste into a suitable file for saving on your PC/Mac.

**WARNING**: Do not replace existing good source with a source from your board until you have confirmed it is a good copy. I strongly suggest you  save into a temporary file first, then check its file length and CRC before replacing a known-good file. To do this use the Info option to find the file's CRC (Cyclic Redundancy Check) and file length. Then check on your PC that the temporary file you saved has the same length and CRC. See above for using Jacksum for calculating CRC values.

---

## Edit

Pressing **E** take you into the [Editor](editor.htm).

---

## Recover

This is designed to help you recover your source in the event that you press Reset. Pressing Reset sets the source to be empty. It does this by writing 0x00 to the first byte of the source area. The rest of the source is untouched.  The Recover option does the following:

* Checks that the first byte of the source is 0x00
* If so it replaces it by a space.

This effectively makes your source available again, with possibly the first byte changed from something else to a space. To avoid having the source corrupted by this process I suggest you start your source with a space, then changing the first byte to a space will not have any effect.

---

## Quit

Pressing **Q** takes you back to the main menu.

---

## More about Cyclic Redundancy Checks (CRCs)

CRC checks are designed to reliably detect burst errors in data communications. The code inside G-Pascal implements CRC-16-CCITT with the parameters as shown below. You can use [Jacksum](https://jacksum.net/en/index.html) or your other favourite CRC generating program to compare the CRC generated by G-Pascal to the CRC of your file on disk.

The parameters to Jacksum mean:

* 16 Bit CRC
* Polynomial $1021 (without the leading bit), that is x^16^ + x^12^ + x^5^ + 1
* Initial value $FFFF
* Mirror neither the input nor the output
* No XOR at the end.

This checksum algorithm is otherwise known as CRC-16-CCITT or CRC-CCITT.

To use Jacksum at the command line you can use these parameters:

```
jacksum -a crc:16,1021,FFFF,false,false,0 -x YOUR_FILE_HERE
```

As you can see from [Ben Eater's video](https://www.youtube.com/watch?v=izG7qT0EpBw) the CRC used here will detect any two-bit errors if they fall within the range of 32751 bits (4903 bytes) of each other. This would certainly be the case for reasonably small programs.

### What to do if the CRC disagrees

First, check that the number of bytes is correct. If it is not then the CRC will certainly be different. It is easy to add an extra line at the start or the end of the source, so check that the number of lines is correct, and if necessary delete any extra blank lines at the start or end.

Then check that the file on your PC/Mac does not have any Pascal reserved words in upper-case. The editor tokeniser changes all reserved words to lower-case, so if they are in upper-case on your PC then the CRC will fail.

If you have loaded a program, and the CRC fails, try compiling or assembling it. Any errors in that process may show you where there are corrupted bytes.

---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## References

* [Cyclic redundancy check - Wikipedia](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
* [How do CRCs work? - Ben Eater (YouTube)](https://www.youtube.com/watch?v=izG7qT0EpBw)
* [Jacksum](https://jacksum.net/en/index.html)


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
