var i : integer;
begin
   lcdclear;
   lcdwrite ("Hello, world!");
   delay (5000);
   lcdclear;
   for i := 1 to 26 do
   begin
     lcdhome;
     lcdwrite (i, " ", hex (i), ": ", chr (i + 64));
     delay (2000);
    end;

    delay (4000);
    lcdclear;
    lcdpos (1, 5);
    lcdwrite ("L2, Col 6")
end.
