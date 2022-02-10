var i : integer;
begin
   lcdclear;
   lcdwrite ("Hello, world!");
   delay (1000);
   lcdclear;
   for i := 1 to 20 do
   begin
     lcdhome;
     lcdwrite (i, " ", hex (i), ": ", chr (i + 64));
     delay (500);
    end;

    delay (1000);
    lcdgotoxy (5, 1);
    lcdwrite ("L2, Col 6")
end.
