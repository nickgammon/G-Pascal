{ Adventure-style game
  Author: Nick Gammon
  Original: August 1982
  Updated: January 2022 (40 years later!)
}

const

false = 0;
true = 1;

inhand = -1;
maxobj = 6;
maxwords = 2;
maxcarry = 3;

{ where an object is }

carried = 1;
notcarried = 2;
nearby = 3;

{ objects }

lamp = 1;
bun = 2;
rod = 3;
ring = 4;
statue = 5;
crown = 6;


var

line : array [100] of char;
object : array [maxobj] of integer;
word, startword, endword: array [maxwords] of integer;
holding, score, turns, obj, ptr,
room, oldroom : integer;

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

procedure sayword (x);
var i : integer;
begin
  for i := startword [x] to endword [x] - 1 do
    write (chr (line [i]));
end;  { sayword }

procedure dontunderstand;
begin
  case turns mod 4 of
    0: writeln ("What?");
    1: writeln ("Pardon?");
    2: writeln ("I don't understand that!");
    3: writeln ("Eh?")
  end
end;  { dontunderstand }

procedure crazy;
begin
  case turns mod 4 of
    0: writeln ("Don't be ridiculous!");
    1: writeln ("Nice try.");
    2: writeln ("I wouldn't!");
    3: writeln ("That's a *very* silly idea!")
  end
end;  { crazy }

procedure force;
begin
  writeln ("An invisible force prevents you from passing.");
end;  { force }

procedure noway;
begin
  writeln ("You cannot go that way.");
end; { noway }

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

procedure describeobject (x);
begin
  case x of
    lamp:   write ("lamp");
    bun:    write ("cream bun");
    rod:    write ("rod");
    ring:   write ("gold ring");
    statue: write ("silver statue");
    crown:  write ("jewelled crown")
  end;
end;  { describeobject }

function getobject (mustbe);
begin
  getobject := false;
  if convertobject then
  begin
    if not here (obj) then
      begin
      write ("I see no ");
      sayword (2);
      writeln (" here.")
      end
    else
    case mustbe of
      carried:
        if carrying (obj) then
          getobject := true
        else
          writeln ("You're not carrying it!");
      notcarried:
        if not carrying (obj) then
          getobject := true
        else
          writeln ("You're already carrying it!")
      else
        getobject := true { it's nearby }
      end  { of case }
  end  { if convertobject succeeded }
end; { getobject }

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

procedure dropit (x);
begin
  holding := holding - 1;
  object [x] := room;
  writeln ("Dropped.");
  if room = 1 then
    if  (inroom (ring))
    and (inroom (statue))
    and (inroom (crown)) then
      room := 0  { finished quest! }
end;  { of dropit }

procedure destroy (x);
begin
  if carrying (x) then
    holding := holding - 1;
  object [x] := 0
end;  { of destroy }

procedure initialize;
var i : integer;
begin
  room := 3;
  oldroom := 0;
  score := 0;
  holding := 0;
  turns := 0;
  for i := 0 to maxobj do
    object [i] := 0;
  { place objects in rooms }
  object [lamp] := 1;
  object [bun] := 6;
  object [rod] := 9;
  object [ring] := 8;
  object [statue] := 12;
  object [crown] := 21;
end;  { initialize }

procedure instructions;
begin
  writeln ("Your quest is to explore the cave of the evil Ur-Lord, and bring back to");
  writeln ("the edge of the cliff the following valuables:");
  writeln;
  writeln ("  1. The white gold ring;");
  writeln ("  2. The sacred silver statue;");
  writeln ("  3. The jewelled crown of the Ur-Lord.");
  writeln;
  writeln ("Be careful ...");
  writeln;
  writeln ("Hints: LOOK, INVENTORY, HELP, SCORE,");
  writeln ("       TAKE and DROP, compass directions to move.");
  writeln ("       (Also: IN, OUT, UP, DOWN, INSPECT, WAVE)");
  writeln
end;  { instructions }

procedure describeroom;
  var i : integer;
  begin
  if (room > 4) and (not here (lamp)) then
    writeln ("It's too dark to see!")
  else
  begin
    write ("You are ");
    case room of
      1: writeln ("at a plateau near a cliff. A rocky path leads south.");
      2: writeln ("on a rocky path leading north and curving to the east.\n",
                  "There is a slight breeze.");
      3: writeln ("at the entrance to a dark cave. The cave is east of here. A rocky path to the west curves north.");
      4: writeln ("just inside a dark cave. Light comes from an entrance to the west.\n",
                  "There is a dank, mouldy smell. A tunnel leads south.");
      5: writeln ("in a low north/south tunnel.");
      6: writeln ("in an oval cavern with an exit to the north. There is a forbidding stone staircase leading down.");
      7: writeln ("in a high, square cave with walls of frozen ice. There are passages in many directions\n",
                  "and a stairway leading up.");
      8: writeln ("in a triangular side-chamber.");
      9: writeln ("in a musty-smelling alcove.");
      10: writeln ("in an eerie chamber - small squealing sounds come from the walls.");
      11: writeln ("passing through an enormouse cave with a double pillar of green stone down the centre.\n",
                   "A big arch leads north and dank tunnels lead from the southeast and southwest corners.");
      12: writeln ("crouched in a malodorous tunnel. The only exit is the way you came.");
      13: writeln ("a room which appears to only have an exit in the northwest corner. However you get the\n",
                  "impression that others have somehow travelled onwards through this room - the exact\n",
                  "method they used is not apparent.");
      14: writeln ("a secret room reached only by magic means. A high passage exits to the northwest.");
      15: writeln ("a depressing octagonal room. Eerie passages lead north and southwest.");
      16: writeln ("an enormous misty cavern - the roof is so high that mist obscures it. Passages lead north and south.");
      17: writeln ("a tiny box-shaped room. A door leads south and stairs lead down.");
      18: writeln ("a bizarre room with a smell of chlorine. A path leads north and stairs lead up.");
      19: writeln ("a steamy chamber with warm walls. Footsteps in the dust seem to lead west, and come from the south.");
      20: writeln ("a large room, littered with alabaster slabs. Doors lead east and west.");
      21: writeln ("the throne room of the evil Ur-Lord! A low door leads east.")
end; { of case }

  for i := 0 to maxobj do
    if inroom (i) then
      begin
      write ("There is a ");
      describeobject (i);
      writeln (" here!")
      end;  { of if in room }
  end;  { of for }
  oldroom := room
end;  { of describeroom }

procedure north;
begin
  case room of
    1: crazy;
    2: room := 1;
    5: room := 4;
    6: room := 5;
    7: room := 9;
   11: room := 7;
   15: room := 16;
   16: if carrying (ring) then
         force
       else
         room := 17;
   18: if carrying (statue) then
         room := 19
        else
          force
    else noway end
end;  { of north }

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

procedure east;
begin
  case room of
    2: room := 3;
    3: room := 4;
    7: room := 10;
    8: room := 7;
   20: room := 19;
   21: room := 20
  else noway end
end;  { of east }

procedure west;
begin
  case room of
    1: crazy;
    3: room := 2;
    4: room := 3;
   10: room := 7;
    7: room := 8;
   19: room := 20;
   20: room := 21
    else noway end
end;  { of west }

procedure up;
begin
  case room of
     7: room := 6;
    18: room := 17
    else noway end
end;  { of up }

procedure down;
begin
  case room of
     6: room := 7;
    17: room := 18
    else noway end
end;  { of down }

procedure northeast;
begin
  case room of
    12: room := 11;
    14: room := 15
    else noway end
end;  { of northeast }

procedure northwest;
begin
  case room of
    13: room := 11
    else noway end
end;  { of northwest }

procedure southeast;
begin
  case room of
    11: room := 13
    else noway end
end;  { of southeast }

procedure southwest;
begin
  case room of
    11: room := 12;
    15: room := 14
    else noway end
end;  { of southwest }

procedure in;
begin
  case room of
    3: room := 4
    else noway end
end;  { of in }

procedure out;
begin
  case room of
    4: room := 3
    else noway end
end;  { of out }


procedure getword (x);
var i : integer;
begin
  word [x] := 0;
  i := 0;
  while line [ptr] = " " do
    ptr := ptr + 1;
  startword [x] := ptr;
  while (line [ptr] <> "\n") and
        (line [ptr] <> " ") do
  begin
    if word [x] and $ff0000 = 0 then
      word [x] := word [x] + (line [ptr] shl i);
    i := i + 8;
    ptr := ptr + 1
  end;
  endword [x] := ptr
end; { getword }

procedure getline;
var i : integer;
begin
  writeln;
  repeat
    read (line);
    line [99] := "\n";  { force newline at end }
    ptr := 0;
    for i := 1 to maxwords do
      getword (i);
    while line [ptr] = " " do
      ptr := ptr + 1;
    if line [ptr] <> "\n" then
    begin
      word [1] := 0;
      writeln ("Please use no more than ", maxwords, " words\n")
    end
  until word [1] <> 0
end;  { getline }

procedure ok;
begin
  writeln;
  writeln ("OK.")
end;  { ok }

procedure take;
var i : integer;
begin
  if word [2] = "all" then
    for i := 0 to maxobj do
      if inroom (i) then
      begin
        describeobject (i);
        write (": ");
        pickup (i)
      end else
   else  { not all }
   if getobject (notcarried) then
      pickup (obj)
end;  { of take }

procedure drop;
var i : integer;
begin
  if word [2] = "all" then
    for i := 0 to maxobj do
      if carrying (i) then
      begin
        describeobject (i);
        write (": ");
        dropit (i)
      end else
   else  { not all }
   if getobject (carried) then
      dropit (obj)
end; { of drop }

procedure givescore;
begin
  score := 0;
  if object [statue] = 1 then
    score := score + 100;
  if object [ring] = 1 then
    score := score + 100;
  if object [crown] = 1 then
    score := score + 100;
  if carrying (statue) then
    score := score + 100;
  if carrying (ring) then
    score := score + 100;
  if carrying (crown) then
    score := score + 100;
  writeln ("\nYour score is ", score, " points, in ",
           turns, " turns.")
end;  { givescore }

procedure quit;
var reply: char;
begin
  givescore;
  write ("Do you want to quit now? y/n ");
  read (reply);
  if reply <> "y" then
    ok
  else
    room := 0
end; { quit }

procedure wave;
begin
  if getobject (carried) then
    if obj = rod then
    begin
      case room of
        13: room := 14;
        14: room := 13
      else
        writeln ("Nothing happens here.")
      end
    end else
      begin
      write ("Waving the ");
      sayword (2);
      writeln (" is not very rewarding!");
      end;
  if room <> oldroom then
    writeln ("There is a blinding flash of light, a loud burping noise,\n",
             "and you suddenly find ...")
end; { wave }

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

procedure inventory;
var i, count : integer;
begin
  count := 0;
  for i := 0 to maxobj do
    if carrying (i) then
    begin
      if count = 0 then
        writeln ("You are carrying:");
      count := count + 1;
      write ("  ");
      describeobject (i);
      writeln;
    end;
  if count = 0 then
    writeln ("You aren't carrying anything!");
end; { inventory }


procedure inspect;
begin
  if getobject (nearby) then
    case obj of
      rod, ring, statue:
        begin
        write ("Magic seems to emanate from the ");
        sayword (2);
        writeln (" ...");
        end;
       bun: writeln ("It looks tasty!")
    else
      writeln ("You see nothing special.")
    end  { of case }
end;  { inspect }

procedure look;
begin
  if word [2] = 0 then
    oldroom := 0  { force look of room }
  else
    inspect { inspect whatever-it-is }
end;  { look }


procedure verb;
var i : integer;
begin
  writeln;
  i := 1;
  case word [1] of
    "go", "run", "wal", "mov", "cra" :
      begin
        if word [2] = 0 then
        begin
          sayword (1);
          writeln (" where?");
          i := 0;
        end else
          i := 2
      end
  end; { of case }
  if i <> 0 then
  case word [i] of
    "qui", "q" : quit;
    "eat"      : eat;
    "nor", "n" : north;
    "sou", "s" : south;
    "eas", "e" : east;
    "wes", "w" : west;
    "ne"       : northeast;
    "nw"       : northwest;
    "se"       : southeast;
    "sw"       : southwest;
    "up", "u"  : up;
    "dow", "d" : down;
    "in"       : in;
    "out"      : out;
    "inv", "i" : inventory;
    "loo", "l" : look;
    "wav"      : wave;
    "ins"      : inspect;
    "tak", "t", "get", "pic" : take;
    "dro", "put", "pla", "thr" : drop;
    "h", "hel", "?" : instructions;
    "sco"      : givescore
  else
    dontunderstand
  end { of case }
end;  { verb }

{ ---------------------
  PROGRAM STARTS HERE
  -----------------------}

begin
  initialize;
  instructions;
  repeat
   turns := turns + 1;
   if room <> oldroom then
     describeroom;
   getline;
   verb
  until room = 0;
  givescore;
  if score = 300 then
    writeln ("\n\nCongratulations!!\n\n",
             "You have completed your quest!")

end .  { main block }
