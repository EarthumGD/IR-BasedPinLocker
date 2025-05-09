int decodeHexCode(unsigned long code) {
  switch (code) {
    case 3860463360:
      return 0;
    case 3125149440:
      return 1;
    case 3108437760:
      return 2;
    case 3091726080:
      return 3;
    case 3141861120:
      return 4;
    case 3208707840:
      return 5;
    case 3158572800:
      return 6;
    case 4161273600:
      return 7;
    case 3927310080:
      return 8;
    case 4127850240:
      return 9;
    case 3910598400:
      return -1;  // *
    case 4061003520:
      return -2;  // #
    case 3877175040:
      return -3;  // Up
    case 2907897600:
      return -4;  // Down
    case 4144561920:
      return -5;  // Left
    case 2774204160:
      return -6;  // Right
    case 3810328320:
      return -7;  // Ok
    default:
      return -8;  // No input
  }
}