  { Relocate symbol table and runtime stack: } {%s $5800}
  { Note: hello_world_relocated.asm should be loaded and compiled before running this }

begin
  writeln ("About to call machine code ...");
  call ($5800);
  writeln ("Machine code done.")
end .
