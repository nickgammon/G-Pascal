var i : integer;
begin
  pinmode (3, 1);
  for i := 1 to 10 do
    begin
    digitalwrite (3, 1);
    delay (1000);
    digitalwrite (3, 0);
    delay (1000);
    end
end .
