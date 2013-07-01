FNR==NR {
for(i=1; i<=NF; i++)
_[FNR,i]=$i
next
}
{
for(i=1; i<=NF; i++)
printf("%d%s", $i+_[FNR,i], (i==NF) ? "\n" : FS);
}
