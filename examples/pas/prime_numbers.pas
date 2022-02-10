{ Eratosthenes Sieve prime number generator }

{ Note: does not detect the number 2 }

const size = 2000;
      true = 1;
      false = 0;
      perline = 10;

var flags : array [size] of char;
    i, prime, k, count, online : integer;

begin
    count := 0;     { how many primes we found }
    writeln;

    online := 0;    { numbers we have shown on this line }

    for i := 0 to size do
      flags [i] := true;

    for i := 0 to size do
        if flags [i] then
        begin
          prime := i + i + 3;
          k := i + prime;
          while K < size do
          begin
            flags [k] := false;
            k := k + prime
          end;
          if online > perline then
          begin
            writeln;
            online := 0
          end;
          online := online + 1;
          count := count + 1;
          write (prime, " ")
        end;
      writeln;
      writeln (count, "  primes")
end.
