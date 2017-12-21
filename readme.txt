
The buzz hack functions are now in the new machine interface. You can use them and still stay compatible with the old buzz exe if you do this:

if (pCB->GetNearestWaveLevel(-2, -2) != 0 && pCB->GetHostVersion() >= 2)
{
  // use new functions
}
else
{
  // use hacks
}

