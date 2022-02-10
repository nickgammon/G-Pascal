 { 'Guess the numbers' Game
   Author: Nick Gammon
   Date: January 1984
   }

const true = 1;
      false = 0;
      cr = 10;
      numdigits = 4;
      largestdigit = 9;

var ch : char;
    number : array [numdigits] of char;
    number_black,
    number_white,
    guesses : integer;

procedure initialise;
(*******************)
begin
  mem [$F] := mem [$13];  { re-seed random number }
end;

procedure create_number;
(**********************)
var i : integer;
begin
  for i := 1 to numdigits do
    number [i] := "0" + random mod (largestdigit + 1);

  writeln ("I am thinking of a ", numdigits,
           " number with each digit in the range 0 to ",
           largestdigit, ".");
  writeln ("Try to guess the number.");
  writeln ('You will be awarded a white token for the right digit in the right place,');
  writeln ('and a black token for the right digit in the wrong place.')

end;

procedure play;
(*************)
var user : array [numdigits] of char;
    notbad,
    i,
    j : integer;
    used : array [numdigits] of integer;

begin
  number_white := 0;
  number_black := 0;
  write ("Your guess? ");
  read (user);
  notbad := user [numdigits] = cr;
  i := 0;
  while notbad and (i < numdigits) do
    begin
    notbad := (user [i] >= "0") and (user [i] <= "0" + largestdigit);
    i := i + 1
    end;
  if not notbad then
    begin
    writeln ("Illegal input, try again");
    play
    end
  else
    begin
    for i := 1 to numdigits do
      begin
      if number [i] = user [i - 1] then
        begin
        number_white := number_white + 1;
        used [i] := true;
        end
      else
        used [i] := false;
      end;
    for i := 1 to numdigits do
      begin
      j := 1;
      while j < numdigits do
        begin
        if (user [i - 1] = number [j])
        and (i <> j)
        and not used [j] then
          begin
          number_black := number_black + 1;
          used [j] := true;
          j := numdigits
          end;
        j := j + 1
        end
      end
    end
end;  { of play }

begin  { main program }
  initialise;
  repeat
    create_number;
    number_white := 0;
    guesses := 0;
    while number_white <> numdigits do
      begin
      play;
      writeln (number_white, " white, ", number_black, " black.");
      guesses := guesses + 1;
      end;
    writeln;
    writeln ("Correct!");
    writeln ("You took ", guesses, " guesses.");
    writeln;
    write ("Try again? ");
    read (ch);
  until ch  <> "y";
end . { main program }


