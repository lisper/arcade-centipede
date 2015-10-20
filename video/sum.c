
main()
{
	unsigned int l;
	unsigned int h, sum, top, match;

	// 0x3921b710
	// h=b7, v=21
	h = 0x91;
	for (l = 0; l < 256; l++) {
		sum = l + h;
		top =
			((sum & 0x80) >> 7) &
			((sum & 0x40) >> 6) &
			((sum & 0x20) >> 5) &
			((sum & 0x10) >> 4);
		match = top ? 0 : 1;

		printf("line %x (%d): sum %x top %x match %x\n",
		       l, l, sum, top, match);
	}
}
