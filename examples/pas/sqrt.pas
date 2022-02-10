{ Program to demonstrate how to obtain square roots in G-Pascal.
  To improve accuracy, output from SQRT is 128 times the actual result.
  For the correct result, divide by 128 (or shift right 6 times)
}

var j, k : integer;

function sqrt (x);
{----------------}

var sq, a, b, i : integer;
begin

if x = 0 then
  begin
  sqrt := 0;
  end
else
  begin
  x := x shl 12;
  sq := abs (x);
  a := x;
  b := 0;
  i := 0;

  while a <> b do
    begin
    b := sq / a;
    a := (a + b) shr 1;  { divide by 2}
    i := i + 1;
    if i > 4 then
      begin
      i := 0;
      if abs (a - b) < 2 then
        a := b
      end
    end;
  sqrt := a
  end;
end ; { sqrt }

{ Program starts here - display square roots 6of first 200 numbers to 2 decimal places }

begin
  for k := 0 to 200 do
    begin
    j := (sqrt (k) * 100) shr 6;
    write (" sqrt(", k, ") = ", j / 100, ".");
    if j mod 100 < 10 then
      write ("0");
    writeln (j mod 100)
    end
end.
