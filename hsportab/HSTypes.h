struct HSRect
{
  HSRect(int aLeft, int aRight, int aTop, int aBottom)
    : left(aLeft), right(aRight), top(aTop), bottom(aBottom)
  {
  }

  int left, right, top, bottom;
};

enum HSKeyCode
{
  HSKC_NUMPLUS,
  HSKC_NUMMINUS
};