% G-Pascal Editor

**Author**: Nick Gammon

**Date**: February 2022

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Loading and saving](file_menu.htm) </div>


The inbuilt text editor is designed to allow you to try out small programs, and make modifications "on the fly" without having to download or upload code from your "main" computer.

## Editor commands

The editor is available from the main "shell" prompt. Type **H** to see a list of editor commands:

```
Available actions:

List/SAve   line_number_range
Delete      line_number_range
Insert/LOad after_line
Find        line_number_range /target/flags
Replace     line_number_range /target/replacement/flags

Help
INfo
Memory first_address last_address
Compile/Syntax/Assemble
RUn/DEBug/Trace
RECover
(Actions may be abbreviated)
(Flags: 'I'gnore case, 'G'lobal, 'Q'uiet)
```

You can assemble, compile (Pascal) or do a (Pascal) syntax check directly from the prompt (type **A**, **C** or **S**).

The editor is line-based with each line having a number, automatically assigned. These numbers can be used to delete, insert, list, modify, find or replace. You need to press ENTER to have a command processed. Make sure you have "local echo" on in your terminal program (eg. miniterm) so that you can see your typing.

Commands have a line number range (if omitted, they affect all lines). For example to list lines 1 to 10:

```
L1-10
```

You may put spaces after the action, and between the line numbers if giving a range. You can also use a hyphen or comma to separate line numbers, e.g.


```
LIST 1-10
LIST 1 10
LIST 1, 10
```

Actions may be abbreviated. The minimal amount to type is in upper case, so "L" would be LIST and not LOAD, and "S" would be SYNTAX and not SAVE.

As a shortcut, the command R on its own will be interpreted as Run (not Replace) as Replace would always need a delimiter after it.

---

## Delete

Use DELETE to delete a range of lines (you cannot use DELETE on its own to delete the entire source). If you want to delete *everything* type "delete all" (or "d all").

---

## Find

Use FIND to find a string. The string delimiter can be any single character which is not a letter, number or space. This is intended to help you find lines with certain words on them. For example:

```
F .procedure.
```

The matching lines will be shown, with their line numbers, so you could do a List of around that spot.

You may put options after the second delimiter as follows:

  * G - global: find multiple occurrences on one line (this affects the number of matches reported)
  * I - ignore case: match on both upper and lower-case versions of the target string
  * Q - quiet: do not show matching lines, just show the count of matches

For example:

```
F 1, 100 /begin/giq
```

That would search for "begin" between lines 1 and 100, and report on the number of occurrences, case insensitive. The lines would not be listed.

```
F 20 - 999 /begin/
```

That would list all lines with "begin" on them between lines 20 and 999 (or the end of the file, whichever came sooner).

---

## Insert

Use INSERT to insert new lines. Use **I** on its own to insert right at the start of the source. Otherwise if you give a line number the inserted lines will appear after that line. To cancel inserting, press **Esc**.

**WARNING**: If you attempt to insert multiple lines very quickly your source will be corrupted. In particular, this will happen if you go into Insert mode and then paste a whole lot of text from your PC. The reason for this is that displaying the line numbers takes quite a few calculations (binary to decimal conversion), which take time.

If you are planning to "dump" your source onto the board, then use LOAD and "Load" it. This inserts source without displaying the line numbers.

---

## List

Use LIST on its own to list the entire source. Otherwise list one line or a range, for example:
Use LIST on its own to list the entire source. Otherwise list one line or a range, for example:

```
L
L42
L50-60
```

As an alternative, for exporting your file back to your PC/Mac, use SAVE to save the source. This lists it without line numbers, suitable for copying from your terminal program and pasting back into a text editor on your PC.

Press Ctrl+C to abort a long listing.

Control characters in the source are shown with a carat before them. For example if you had 0x01 in your source it would show as ^A if you listed it.

---

## Replace

This lets you replace one string with another, either in a range of lines or the whole source. You might want to rename a variable with this, for example. You need to specify the "find" string and the "replace" string, separated by a delimiter of your choice. The string delimiter can be any single character which is not a letter, number or space.

There are options you can place after the third delimiter:

  * G - global: find and replace multiple occurrences on one line
  * I - ignore case: match on both upper and lower-case versions of the target string
  * Q - quiet: do not show matching lines, just show the count of matches


For example:

```
R .fish.chips.g
R 1-20 /dog/cat/ i
```

---

## Memory

This lets you view any part of your RAM or ROM memory. You enter a start and end address, and the bytes in that range are displayed in hex, and also bytes in the range 0x20 to 0x7F are also displayed in ASCII on the right. eg.

```
: mem $300 $31f
$0300: 7b 20 41 64 76 65 6e 74 75 72 65 2d 73 74 79 6c  {   A d v e n t u r e - s t y l
$0310: 65 20 67 61 6d 65 0a 20 20 41 75 74 68 6f 72 3a  e   g a m e .     A u t h o r :
```

Press Ctrl+C to abort a long listing.


---

## Recommendation

For large-scale changes I suggest keeping a copy of your source on your PC (you need to do this anyway as the board does not have file saving capability). For anything other than small one or two-line changes, it will be quicker and easier to change the source on your PC, and re-download it, rather than fiddling around in the Editor.

The editor does not support cursor keys, so it is not the easiest editor to work with, particularly compared to modern editors available on PC/Macs.

Keeping your original source elsewhere also protects you against rogue code which might overwrite your source, as there is no memory-protection hardware on these chips.



---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
