begin
  pinmode (3, 1);
  pinmode (4, 0);

  repeat
    begin
    digitalwrite (3, digitalread (4));
    end
  until 0
end .
