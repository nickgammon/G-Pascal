  { Melee adjudication game.

    Written by Nick Gammon.

    Originally written in October 1985.

  }

const true = 1;
      false = 0;
      maxchars = 20;
      newline = 10;
      show_monsters = false;
      maxlevel = 9;
      action_no_surprise = 0;
      action_x_surprises_y = 1;
      action_y_surprises_x = 2;
      action_parry = 3;
      action_run = 4;
      player = 1;

var

{ all of the variables here are part of an array }

  hp,  { hit points }
  ac,  { armour class }
  hitdie,
  dexterity,
  tohit,
  damagetimes,
  damagedie,
  damageplus,
  level,
  maxhp,
  constitution,
  hp_adjustment,
  attack_adjustment,
  defence_adjustment,
  experience,
  strength,
  iq,
  wisdom,
  charisma,
  type,
  gold : array [maxchars] of integer;

{ rest of the variables }

  i, j : integer;
  reply : char;

procedure name (x);
begin
  case type [x] of
   player: write ("Human");
   10: write ("Berserker");
   11: write ("Bandit");
   12: write ("Black Pudding");
   13: write ("Bugbear");
   14: write ("Chimera");
   15: write ("Cockatrice");
   16: write ("Doppleganger");
   17: write ("White Dragon");
   18: write ("Black Dragon");
   19: write ("Red Dragon")
  end  { of case }
end; { of name }

function roll (times, die);
var cumulative, i : integer;
begin
  cumulative := 0;
  for i := 1 to times do
    cumulative := cumulative + (random mod die + 1);
  roll := cumulative
end;  { of roll }

procedure displaychar (x);
begin
  write ("---------------- ");
  name (x);
  writeln (" ----------------");
  writeln;
  writeln;
  writeln ("Strength:           ", strength [x]);
  writeln ("IQ:                 ", iq [x]);
  writeln ("Wisdom:             ", wisdom [x]);
  writeln ("Constitution:       ", constitution [x]);
  writeln ("Dexterity:          ", dexterity [x]);
  writeln ("Charisma:           ", charisma [x]);
  writeln ("Max. hit points:    ", maxhp [x]);
  writeln ("Hit points:         ", hp [x]);
  writeln ("Level:              ", level [x]);
  writeln ("Armour class:       ", ac [x]);
  write   ("Roll to hit AC 0:   ",
           damagetimes [x], "d", damagedie [x]);
  if damageplus [x] > 0 then
    write (" + ", damageplus [x]);
  writeln;
  writeln ("Damage:             ", strength [x]);
  writeln ("Experience:         ", strength [x]);
  writeln ("Gold:               ", strength [x]);
  writeln;
  writeln;
end; { displaychar }

procedure generatechar
  (x,      { character number }
   typ,    { type of character }
   lvl,    { level }
   hd,     { hit die }
   armour, { armour class }
   dt,     { damage times }
   dd,     { damage die }
   dp);    { damage plus }

var i, j : integer;
begin
  repeat

    type [x] := typ;
    damagetimes [x] := dt;
    damagedie [x] := dd;
    damageplus [x] := dp;
    ac [x] := armour;
    tohit [x] := 12 - lvl;
    if tohit [x] < 0 then
      tohit [x] := 0;
    experience [x] := 0;

    dexterity [x] := roll (3, 6);

    attack_adjustment [x] := 0;
    case dexterity [x] of
    18: attack_adjustment [x] := 3;
    17: attack_adjustment [x] := 2;
    16: attack_adjustment [x] := 1;
     5: attack_adjustment [x] := -1;
     4: attack_adjustment [x] := -2;
     3: attack_adjustment [x] := -3
    end; { of case }

    defence_adjustment [x] := 0;
    case dexterity [x] of
    18: defence_adjustment [x] := -4;
    17: defence_adjustment [x] := -3;
    16: defence_adjustment [x] := -2;
    15: defence_adjustment [x] := -1;
     6: defence_adjustment [x] := 1;
     5: defence_adjustment [x] := 2;
     4: defence_adjustment [x] := 3;
     3: defence_adjustment [x] := 4
    end; { of case }

    constitution [x] := roll (3, 6);
    hp_adjustment [x] := 0;
    case constitution [x] of
    18: hp_adjustment [x] := 4;
    17: hp_adjustment [x] := 3;
    16: hp_adjustment [x] := 2;
    15: hp_adjustment [x] := 1;
     6: hp_adjustment [x] := -1;
     5: hp_adjustment [x] := -1;
     4: hp_adjustment [x] := -1;
     3: hp_adjustment [x] := -2
    end; { of case }

    iq [x] := roll (3, 6);
    strength [x] := roll (3, 6);
    wisdom [x] := roll (3, 6);
    charisma [x] := roll (3, 6);
    gold [x] := roll (3, 6) * 10;
    hitdie [x] := hd;
    level [x] := lvl;
    maxhp [x] := 0;

    for i := 1 to lvl do
      begin
      j := roll (1, hitdie [x]);
      j := j + hp_adjustment [x];
      if j < 1 then
        j := 1;
      maxhp [x] := maxhp [x] + j;
      end;
      hp [x] := maxhp [x];

      if x = player then
        begin
        displaychar (x);
        writeln;
        write ("Keep this character? Y = Yes ... ");
        read (reply);
        writeln;
        end
      else
        reply := "y";
  until ((reply = "y") or (reply = "Y"))

end; { generatechar }

procedure generatemonster (x);
var y, z : integer;
begin
  z := roll (1, 8); { for dragon's hit dice }
  y := roll (1, 10) + 9;  { monster type }
  case y of
    10: generatechar (x, y,  1, 8, 7, 1,  8, 0);
    11: generatechar (x, y,  1, 8, 6, 1,  6, 0);
    12: generatechar (x, y, 10, 8, 6, 3,  8, 0);
    13: generatechar (x, y,  3, 8, 5, 2,  4, 0);
    14: generatechar (x, y,  9, 8, 4, 3,  4, 0);
    15: generatechar (x, y,  5, 8, 6, 1,  6, 0);
    16: generatechar (x, y,  4, 8, 5, 1, 12, 0);
    17: generatechar (x, y,  5, z, 2, 4,  6, 0);
    18: generatechar (x, y,  6, z, 2, 4,  6, 0);
    19: generatechar (x, y, 11, z, 2, 4,  6, 0)
  end { of case }
end;  { generatemonster }

procedure melee (x, y, action);
{ character x fights character y }
{ action: 1 = x surprises y
          2 = y surprises x
          3 = x parries
          4 = x runs away }

var i, j, bonus : integer;

{ nested procedure: hit }

procedure hit (x, y);
{ character x attacks character y }

var i, j : integer;
begin
  writeln;
  writeln;
  name (x);
  write (" attacks ");
  name (y);
  write (" and ");
  j := 9 - ac [y] + defence_adjustment [y];
  j := J + tohit [x];
  i := roll (1, 20);
  if i >= j - bonus then
    begin
    write ("hits ");
    if damagetimes [x] > 1 then
      writeln (damagetimes [x], " times.")
    else
      writeln ("once.");
    i := roll (damagetimes [x], damagedie [x]) + damageplus [x];
    name (y);
    writeln (" takes ", i, " points damage.");
    hp [y] := hp [y] - i;
    if hp [y] < 0 then
      hp [y] := 0;
    name (y);
    if hp [y] = 0 then
      writeln (" is killed!")
    else
      writeln (" has ", hp [y], " HP remaining!");
    end
  else
    writeln ("misses!")
end; { hit }

{ *** Start of melee procedure *** }

begin
if action = action_no_surprise then { no surprise }
  begin
  bonus := 0;
  i := dexterity [x];
  j := dexterity [y];
  if abs (i - j) < 3 then
    begin
    i := roll (1, 6);
    j := roll (1, 6);
    end;
  if i > j then { first blow? }
    begin
    hit (x, y);  { x hits first }
    if hp [y] > 0 then
      hit (y, x)  { y retaliates }
    end
  else
    begin
    hit (y, x);  { y hits first }
    if hp [x] > 0 then
      hit (x, y)  { x retaliates }
    end
  end  { no surprise }
else
  if action = action_x_surprises_y then
    begin   { x surprises y }
    bonus := 2;
    hit (x, y);
    end
  else
  if action = action_y_surprises_x then
    begin   { y surprises x }
    bonus := 2;
    hit (y, x);
    end
  else
  if action = action_parry then
    begin   { x parries }
    bonus := -2;
    hit (y, x);
    end
  else
  if action = action_run then
    begin   { x runs }
    bonus := 2;
    hit (y, x);
    end
end;  { melee }

procedure encounter (y);
var action, ok_reply : integer;
begin
  if hp [y] > 0 then
    begin
    writeln;
    writeln;
    write ("** An encounter with a ");
    name (y);
    write (" ");
    for i := 1 to 20 do
      write ("*");
    writeln;
    writeln;
    action := action_no_surprise;
    i := roll (1, 6); { x surprise y? }
    if i <= 2 then
      action := action_x_surprises_y;
    i := roll (1, 6); { y surprise x? }
    if i <= 2 then
      if action = action_x_surprises_y then
        action := 0  { cannot both be surprised }
      else
        action := action_y_surprises_x;
    if action = action_x_surprises_y then
      writeln ("You surprised the monster!")
    else
    if action = action_y_surprises_x then
      writeln ("The monster surprised you!");
    repeat
      if action = action_no_surprise then
        begin
        repeat
          writeln;
          writeln;
          write ("<F>ight, <I>nfo, <P>arry, <R>un ... ");
          read (reply);
          writeln;
          if (reply = "i") or (reply = "I") then
            begin
            displaychar (player);
            displaychar (y)
            end;
          case reply of
            "f", "F", "p", "P", "r", "R" : ok_reply := true
          else
            ok_reply := false
          end { of case }
        until ok_reply;
        case reply of
          "f", "F": action := action_no_surprise;
          "p", "P": action := action_parry;
          "r", "R": action := action_run
        end { of case }
      end;
      melee (1, y, action);
      if action <> action_run then
        action := action_no_surprise;
    until (action = action_run)
       or (hp [player] = 0)  { player dead }
       or (hp [y] = 0)  { monster dead }
    end
end; { encounter }

{ START OF MAIN PROGRAM }

begin { main block }
repeat
  randomseed (latency);   { prime random numbers }

  repeat
    writeln ("What level for human? (1 to ", maxlevel, ") ");
    read (i)
  until (i >= 1) and (i <= maxlevel);

  generatechar (player, 1, i, 6, 9, 1, 6, 0);
  writeln;
  writeln ("Generating monsters ...");
  for i := 2 to 10 do
    generateMonster (i);

  if show_monsters then
    for i := 2 to 10 do
      begin
      displaychar (i);
      writeln ("Press a key ...");
      repeat until getkey
      end;

  { do the battle }

  repeat
    encounter (roll (1, 9) + 1)
  until hp [1] <= 0;
  writeln;
  write ("<Q>uit or <N>ext game: ");
  read (reply);
  writeln
until ((reply = "q") or (reply = "Q"));

end.
