  { Expression evaluator test }
  {%L}

begin

  assert ( 0 = not 1    );   {  not }
  assert ( 1 = not 0    );   {  not }
  assert ( 0 = not 666  );   {  not }
  assert ( -1 = - (1) );   {  unary minus }
  assert ( $1234 shl 8 = $123400  );   {  shift left }
  assert ( $1234 shr 4 = $123     );   {  shift right }
  assert ( $ABCDEF and $00FF00 = $00CD00  );   {  bitwise AND }
  assert ( $FFFFFF xor $101010 = $EFEFEF  );   {  bitwise xor }
  assert ( $000000 or $101010 = $101010  );   {  bitwise or }
  assert ( 524288 * 2 = 1048576  );   {  multiplication }
  assert ( 1048576 / 2 = 524288  );   {  division }
  assert ( 1235684 mod 3 = 2       );   {  modulus (remainder) }
  assert ( 2 + 2 = 4                  );   {  addition lol }
  assert ( 1235684 + 524288 = 1759972 );   {  addition }
  assert ( 1235684 - 524288 = 711396  );   {  subtraction }
  assert ( 42 < 666               );   {  less than }
  assert ( 42 <= 42               );   {  less than or equal }
  assert ( 1234567 >  1234566    );   {  greater than }
  assert ( 1234567 >= 1234567   );   {  greater than or equal }
  assert ( 2 <> 3  );   {  unequal }
  {   Precedence tests }
  assert ( 2+3*4 = 14 );   {  not 20 }
  assert ( - 5 + 6 = 1 );   {  not -11 }
  assert ( - (5 + 6) = -11 );

  writeln ("No problems.")
end .
