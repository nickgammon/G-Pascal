% G-Pascal File management

**Author**: Nick Gammon

**Date**: February 2022

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Text editor](editor.htm) </div>

As described in the [editor](editor.htm) section, simply entering Insert mode and dumping a whole lot of text into the Editor will not work, because the Editor tries to display line numbers for each line, and will fall behind the rate of the incoming source. Instead use Load as described below, and Save to get a copy of the current source without line numbers. There is also  a Recover function, in case you press Reset and lose your source.

Available actions are:

```
Available actions:

List/SAve   line_number_range
Delete      line_number_range
Insert/LOad after_line
Find        line_number_range /target/flags
Replace     line_number_range /target/replacement/flags

Help
INfo
Memory      first_address last_address
Compile/Syntax/Assemble
RUn/DEBug/Trace
RECover
(Actions may be abbreviated)
(Flags: 'I'gnore case, 'G'lobal, 'Q'uiet)
```

Press **H** to see the above.

---

## Load

This loads your source into memory basically copying it as quickly as possible into memory as it is received. You can specify a line number to load from (that is, to insert into an existing file). Without a line number the incoming source is inserted at the start, otherwise it is inserted after the nominated line.

You will see:

```
Paste source, terminate with Esc
```

Now is the time to copy the source from your Mac/PC, and then use the Paste function in your terminal program (or "send file" if there is such a function). Once the file has been sent press **Esc** to exit the loading mode.

*Do not type anything while the file is loading! That will corrupt it. Just wait until it has finished.*

Once you have finished loading you can type INFO to see something like this:

```
Source starts at $0300
Source ends   at $3a63
Source lines:  645
Source length: 14178 bytes
Source CRC       $efdf
```

You are then told the source length (14178 bytes) and Cyclic Redundancy Check (CRC) which is $efdf. This can be used to verify that the source loaded without errors. In this case I am using "Jacksum" to do the CRC on my PC.

```
$ jacksum -a crc:16,1021,FFFF,false,false,0 -x adventure.pas

efdf	14178	adventure.pas
```

You can see that both the file length (14178) and the checksum ($efdf) agree, therefore the file loaded reliably.

You can re-check the CRC lat any time by using the INFO action. Of course, the CRC will change as you change the source.

---

## Info

As described above under LOAD, typing INFO will tell you where the source starts and ends in memory, its length, and its CRC.

Note that the ending address of the source includes a trailing 0x00 byte, so even an empty file will occupy one byte of memory, however the "length" reported by INFO will not include that byte, to be consistent with the file size reported on your PC.

---

## Save

Save works similarly to List in the Editor, however line numbers are not shown at the start of each line. The intention here is to use Save to simply make a copy of your source, which you can then select in your terminal program, copy, and paste into a suitable file for saving on your PC/Mac.

*Do not type anything while the file is saving! The interrupts that keyboard input generate will corrupt the saved text. Just wait until it has finished.*

**WARNING**: Do not replace existing good source with a source from your board until you have confirmed it is a good copy. I strongly suggest you  save into a temporary file first, then check its file length and CRC before replacing a known-good file. To do this use the INFO action to find the file's CRC (Cyclic Redundancy Check) and file length. Then check on your PC that the temporary file you saved has the same length and CRC. See above for using Jacksum for calculating CRC values.


---

## Recover

This is designed to help you recover your source in the event that you press Reset. Pressing Reset sets the source to be empty. It does this by writing 0x00 to the first byte of the source area. The rest of the source is untouched.  The Recover option does the following:

* Checks that the first byte of the source is 0x00
* If so it replaces it by a space.

This effectively makes your source available again, with possibly the first byte changed from something else to a space. To avoid having the source corrupted by this process I suggest you start your source with a space, then changing the first byte to a space will not have any effect.

---

## More about Cyclic Redundancy Checks (CRCs)

CRC checks are designed to reliably detect burst errors in data communications. The code inside G-Pascal implements CRC-16-CCITT with the parameters as shown below. You can use [Jacksum](https://jacksum.net/en/index.html) or your other favourite CRC generating program to compare the CRC generated by G-Pascal to the CRC of your file on disk.

To use Jacksum at the command line you can use these parameters:

```
jacksum -a crc:16,1021,FFFF,false,false,0 -x YOUR_FILE_HERE
```

The parameters to Jacksum mean:

* 16 Bit CRC
* Polynomial $1021 (without the leading bit), that is x^16^ + x^12^ + x^5^ + 1
* Initial value $FFFF
* Mirror neither the input nor the output
* No XOR at the end.

As you can see from [Ben Eater's video](https://www.youtube.com/watch?v=izG7qT0EpBw) the CRC used here will detect any two-bit errors if they fall within the range of 32751 bits (4903 bytes) of each other. This would certainly be the case for reasonably small programs. Also all odd number of bit errors are detected. In fact CRC-16 should detect errors in the fraction (1 - 2^-16^) of cases, that is, 99.9984% of cases. Note that the figure 32751 is derived from 32767 - 16, where 16 are the 16 bits in the CRC itself.

### What to do if the CRC disagrees

First, check that the number of bytes is correct. If it is not then the CRC will certainly be different. It is easy to add an extra line at the start or the end of the source, so check that the number of lines is correct, and if necessary delete any extra blank lines at the start or end.

If you have loaded a program, and the CRC fails, try compiling or assembling it. Any errors in that process may show you where there are corrupted bytes.

If you cannot resolve the difference try loading or saving the same file again. Also try removing trailing spaces which are hard to spot in the source, and may make the versions on your PC and Ben's Board different. The source in G-Pascal always has a trailing newline at the end of every line (including the last) so make sure your file on disk ends with a newline.

The editor discards carriage-returns (0x0D) when accepting input. You should make sure that your file on disk ends with newlines only (0x0A) and not carriage-return (0x0D) as well. Modern editors have methods of converting files to newline-only endings. Personally I use [Geany](https://www.geany.org/).

---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Text editor](editor.htm) </div>


---

## References

* [Cyclic redundancy check - Wikipedia](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
* [How do CRCs work? - Ben Eater (YouTube)](https://www.youtube.com/watch?v=izG7qT0EpBw) (Video)
* [Jacksum](https://jacksum.net/en/index.html)
* [CRC16-CCITT](http://srecord.sourceforge.net/crc16-ccitt.html)
* [Geany text editor](https://www.geany.org/)

---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
