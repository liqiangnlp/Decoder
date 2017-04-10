while(<STDIN>)
{
  s/[\r\n]//g;
  s/\s+$//g;
  s/^\s+//g;
  @words = split /\t/, $_;
  if ($words[0] eq "EN")
  {
    print STDOUT $words[1]."\n";
  }
}