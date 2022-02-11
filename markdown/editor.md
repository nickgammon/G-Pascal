% G-Pascal Editor

**Author**: Nick Gammon

**Date**: February 2022

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [File menu](file_menu.htm) </div>


The inbuilt text editor is designed to allow you to try out small programs, and make modifications "on the fly" without having to download or upload code from your "main" computer.

## Editor commands

You enter the editor by typing **E** at the main menu, after which you will see a colon prompt. Type **H** to see a list of editor commands:

```
The commands are :

 <A>ssemble
 <C>ompile
 <D>elete line number range
 <F>ind   line number range . string .
 <I>nsert line number (Terminate input with Ctrl+D)
 <L>ist   line number range
 <M>odify line number range
 <Q>uit
 <R>eplace line number range .old.new.options (options: i/q/g)
 <S>yntax
```

You can assemble, compile (Pascal) or do a (Pascal) syntax check directly from the Editor (type **A**, **C** or **S**).

The editor is line-based with each line having a number, automatically assigned. These numbers can be used to delete, insert, list, modify, find or replace. You need to press ENTER to have a command processed. Make sure you have "local echo" on in your terminal program (eg. miniterm) so that you can see your typing.

Commands have a line number range (if omitted, they affect all lines). For example to list lines 1 to 10:

```
L1-10
```

Do not put spaces before or after the "-" symbol.


---

## Delete

Use **D** to delete a range of lines (you cannot use **D** on its own to delete the entire source). If you want to delete *everything* just choose a big range, eg.

```
D1-9999
```

If you attempt to delete five or more lines you will be asked for confirmation, as a safety measure, in case you accidentally mistype the line number range.

---

## Find

Use **F** to find a string. The string delimiter is fixed as a period. This is intended to help you find lines with certain words on them. For example:

```
F.procedure.
```

The matching lines will be shown, with their line numbers, so you could do a List of around that spot.


---

## Insert

Use **I** to insert new lines. Use **I** on its own to insert right at the start of the source. Otherwise if you give a line number the inserted lines will appear after that line. To cancel inserting, press Ctrl+D.

**WARNING**: If you attempt to insert multiple lines very quickly your source will be corrupted. In particular, this will happen if you go into Insert mode and then paste a whole lot of text from your PC. The reason for this is that source is tokenised "on the fly" to save memory. This involves:

* Replacing two or more spaces by DLE/count (that is: 0x10 nn where nn is the number of spaces). The count has the high-order bit set to avoid confusing a run of 10 spaces with a newline.
* Replacing reserved words by a single byte. There is a list of reserved words on the [Pascal compiler](pascal_compiler.htm) page.

This tokenisation takes time. By the time a line has been tokenised more data has arrived from the serial port and been ignored.

If you are planning to "dump" your source onto the board, then use the [File menu](file_menu.htm) and "Load" it. This inserts source differently:

* The source is copied into memory as quickly as possible
* When the loading is complete the source is tokenised afterwards in a batch (this may take a second or two).

---

## List

Use **L** on its own to list the entire source. Otherwise list one line or a range, for example:

```
L
L42
L50-60
```

As an alternative, for exporting your file back to your PC/Mac, use the File menu and Save the source. This basically lists it without line numbers, suitable for copying from your terminal program and pasting back into a text editor on your PC.

Press Ctrl+C to abort a long listing.

---

## Modify

Modify:

* Lists a line or range of lines
* Delete the listed lines
* Enters insert mode so you can replace them

The intention here is to correct minor typos, particularly on a small number of lines. Since the lines are echoed as part of the modify process you can see what they used to contain, and retype them correctly, or use copy/paste on your PC to copy parts of them back. Since this effectively involves deleting the lines you are prompted if you attempt to modify five or more lines for confirmation.


---

## Quit

This returns you to the Main menu. Alternatively type **QF** to go directly to the [File menu](file_menu.htm). You would need to be in the Main menu to Run your code.

---

## Replace

This lets you replace one string with another, either in a range of lines or the whole source. You might want to rename a variable with this, for example. You need to specify the "find" string and the "replace" string, separated by periods. There are options you can place after the second period:

* i : ignore case (case-insensitive find)
* q : quiet (do not list lines which are replaced)
* g : global (do multiple replacements on a single line if possible)

For example:

```
R.fish.chips.g
R1-20.dog.cat.i
```


---

## Recommendation

For large-scale changes I suggest keeping a copy of your source on your PC (you need to do this anyway as the board does not have file saving capability). For anything other than small one or two-line changes, it will be quicker and easier to change the source on your PC, and re-download it, rather than fiddling around in the Editor.

The editor does not support cursor keys, backspacing, and so on, so it is not the easiest editor to work with, particularly compared to modern editors available on PC/Macs.

Keeping your original source elsewhere also protects you against rogue code which might overwrite your source, as there is no memory-protection hardware on these chips.

---

## Tokenisation quirks

* Reserved words will always be displayed in lower-case, regardless of how you entered them
* As reserved words are stored internally as one byte, the Find and Replace commands in the Editor *cannot locate part of a reserved word*. For example, you cannot find BEG in BEGIN, or PROC in PROCEDURE.
* It is permissable to locate part of a non-reserved word, unless that part is itself a reserved word. In other words, attempting to locate FR will successfully locate the word FRED, however trying to locate TO will *not* locate the word TOOL as TO is a reserved word.
* As multiple spaces are stored as a two-byte code, the Find and Replace commands can only match the *exact* number of spaces. Remember that the space which is displayed following a reserved word is not actually stored in the file and should not be counted.
* A line with mismatched quote symbols may display strangely. Quotes are detected in the display process, and reserved words are not expanded inside quotes.


---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
