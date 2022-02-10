% G-Pascal Adventure

**Author**: Nick Gammon

**Date**: Original text written in 1982

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>

![](images/Pascal Adventure.png)

Ever since Will Crowther and Don Woods wrote in 1976 the "original" Adventure game, known as Colossal Cave Adventure, computerised fantasy games have become very popular. These days, of course, you can play in highly sophisticated immersive 3D worlds, such as World of Warcraft, and Final Fantasy XIV.

Have you not wanted to write your *own* game? To design a dungeon, place treasures, make mazes, and generally be in control? With G-Pascal you can do that with a minimum of fuss.

### Why Pascal?

Pascal makes the job easy because:

* Its long data names make the program self-documenting.
* The **case** statement is very useful for decision-making.
* Its strong structure makes it easy to debug and enhance.

---

### Source code

The game is available [here](examples/pas/adventure.pas). Because of its size you need to make the hardware modification described [here](hardware_mods.htm), to give yourself 24kB of RAM rather than 16kB.

---

### How to play

Adventure games generally accept commands from the player in the form of two-word sentences, for example GET ROD or WALK NORTH. Our game expects commands to consist of a verb, followed in some cases by a noun. For the sake of brevity commands such as GO NORTH can be abbreviated to the direction only: NORTH (for example).

---

### Code structure

Commands are accepted from the player by **getline** which reads a line of text from the player. This calls **getword** twice to decode the text into two words. If **getline** finds more than two words it asks the player to try again. You could easily accept more than two words by changing the constant **maxwords**.

**getword** may seem a bit obscure, but it pasically "packs" the first three letters of the next word on the input line into a single integer (G-Pascal uses 3-byte integers so this is a neat way of storing a three-letter word).

A side-effect of this is that our game only regards the first three characters of the word as significant. **getword** also keeps track of where the whole word starts and ends, so if you take TAKE ANTELOPE it can reply I SEE NO ANTELOPE HERE rather than I SEE NO ANT HERE.

Our adventure player can take one of five general categories of actions: Move (for example, GO NORTH); take something (TAKE ROD); drop something (DROP LAMP); use something (WAVE RING); and other (SCORE, QUIT, INVENTORY).

---

### Make a move ...

The player's current location is stored in **room**.  Their previous location is stored in **oldroom**. If **room** and **oldroom** differ the program prints a description of the current location by using a **case** statement in **describeroom**.

```
    write ("You are ");
    case room of
      1: writeln ("at a plateau near a cliff. A rocky path leads south.");
      2: writeln ("on a rocky path leading north and curving to the east.",chr(newline),
                  "There is a slight breeze.");
      3: writeln ("at the entrance to a dark cave. The cave is east of here. A rocky path to the west curves north.");

...
```

**describeroom** also prints a description of all objects in the same room as the player:

```
  for i := 0 to maxobj do
    if inroom (i) then
      begin
      write ("There is a ");
      describeobject (i);
      writeln (" here!")
      end;  { of if in room }
  end;  { of for }
```

When the player enters a movement command such as NORTH, SOUTH, UP, DOWN and so on, the **verb** procedure calls the appropriate movement procedure. The movement procedures handle all movement in a single **case** statement (per verb) --- invalid directions are caught by the **else** clause of the **case**.

```
procedure south;
begin
  case room of
    1: room := 2;
    4: room := 5;
    5: room := 6;
    9: room := 7;
    7: room := 11;
   16: room := 15;
   17: room := 16;
   19: if carrying (statue) then
         room := 18
        else
          force
    else noway end
end;  { of south }
```

Movement can be conditional --- in the above example the player can only move from room 19 to 18 if carrying the statue.

To add more rooms to our game we merely have to add their descriptions to **describeroom**, and make provision for getting to and from them in the movement procedures.

---

### Manipulating objects

A lot of the fun in adventure games is finding "objects" (such as lamps and rods) and discovering a use for them. An object can either be carried, lying around somewhere, or nonexistent.

In our game we use an array, **object**, to contain the current location of each object. For example **object [lamp]** is the location of the lamp. The locations are: -1: being carried, 0: nowhere, room number: lying in that room.

We now define some handy boolean functions which tell us if a given object is being carried, is in the room, is is here (meaing carried or in the room).

```
function carrying (x);
begin
  carrying := object [x] = inhand
end; { carrying }

function inroom (x);
begin
  inroom := object [x] = room
end;  { inroom }

function here (x);
begin
  here := carrying (x) or inroom (x)
end;  { here }
```

We can now pick up an object by setting its location to -1, or drop it by setting its location to the current room number, for example:

```
procedure pickup (x);
begin
  if holding >= maxcarry then
    writeln ("You can't carry any more!")
  else
  begin
    holding := holding + 1;
    object [x] := inhand;
    writeln ("Taken.")
  end
end;  { of pickup }
```

At this point you might ask: "if I say TAKE LAMP, how does the word 'LAMP' become a subscript in the **object** array?"

Good question! This conversion is carried out by **convertobject**:

```
function convertobject;
begin
  obj := 0;
  if word [2] = 0 then
  begin
    sayword (1);
    writeln (" what?")
  end else
  case word [2] of
    "lam"         : obj := lamp;
    "bun", "cre"  : obj := bun;
    "rod"         : obj := rod;
    "rin", "gol"  : obj := ring;
    "sil", "sta"  : obj := statue;
    "jew", "cro"  : obj := crown
  else
    obj := 9999
  end; { case }
  convertobject := obj
end;  { convertobject }
```

**convertobject** uses a **case** statement to convert the name of any object (or its synonym/s) into a value which is stored in **obj**. (The actual numbers are stored as constants at the start of the program to avoid confusion and make the program more self-explanatory). **convertobject** also prints "WHAT?" if the player has not supplied a noun, so if you just take TAKE, it will reply TAKE WHAT?

**convertobject** is called by the boolean function **getobject** whose function is to check that the requested object is in the right place.

Since **getobject** returns true or false, you can write (for example): "if getobject (carried) then statement;".

In this case the statement is executed if:

* a noun was supplied
* it is a valid object; and
* it is being cararied

If the statement is executed then the the object number is in **obj** otherwise the appropriate error message will already have been printed. This greatly simplifies programming the rest of the game. For example, here is how we handle the verb EAT:

```
procedure eat;
begin
  if getobject (nearby) then
    if obj = bun then
    begin
      writeln ("Thanks! You were rather hungry!");
      destroy (bun)
    end else
      crazy
end; { eat }
```

---


### To add more objects ...

Here's how you add more objects to the game:

* Increase **maxobj** to the appropriate size
* Allocate an internal name and sequential number to each object in the **const** declaration at the start of the program.
* Put the description of each object in **describeobject**
* Put the external name to internal name conversion in **convertobject**
* Allocate the object's initial room number (if any) in **initialize**
* Decide what the use of the object will be and put it in an appropriate procedure. For example, a book might be read, or a key might open a lock.


---

### Expanding the program

To turn  this game into a full-scale adventure you would need to:

* Create more rooms
* Create more objects
* Create more verbs (for example, READ, LOCK, OPEN, magic words and so on).


---


## Example output from the Adventure game:


```
Your quest is to explore the cave of the evil Ur-Lord, and bring back to
the edge of the cliff the following valuables:

  1. The white gold ring;
  2. The sacred silver statue;
  3. The jewelled crown of the Ur-Lord.

Be careful ...

Hints: LOOK, INVENTORY, HELP, SCORE,
       TAKE and DROP, compass directions to move.
       (Also: IN, OUT, UP, DOWN, INSPECT, WAVE)

You are at the entrance to a dark cave. The cave is east of here. A rocky path to the west curves north.

west

You are on a rocky path leading north and curving to the east.
There is a slight breeze.

north

You are at a plateau near a cliff. A rocky path leads south.
There is a lamp here!

take lamp

Taken.

inventory

You are carrying:
  lamp
```

---

### References

[Colossal Cave Adventure: Wikipedia](https://en.wikipedia.org/wiki/Colossal_Cave_Adventure>)

Graphic from Australian *Your Computer* magazine, August 1982, used with permission.

---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
