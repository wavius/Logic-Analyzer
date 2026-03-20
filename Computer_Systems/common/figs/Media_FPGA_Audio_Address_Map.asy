include "..\..\..\Internal\Documentation\Asymptote\Asymptote_UP_Common.asy";
include "..\..\..\Internal\Documentation\Asymptote\Address_Map_Functions.asy";

UPAddressMapHeader( "31 \ldots 24", 0, 2);
UPAddressMapHeader( "23 \ldots 16", 2, 2);
UPAddressMapHeader( "15 \ldots 10", 4, 2);
UPAddressMapHeader( "9", 6, 1);
UPAddressMapHeader( "8", 7, 1);
UPAddressMapHeader( "7 \ldots 4", 8, 2);
UPAddressMapHeader( "3", 10, 1);
UPAddressMapHeader( "2", 11, 1);
UPAddressMapHeader( "1", 12, 1);
UPAddressMapHeader( "0", 13, 1);

UPAddressMapField( "Unused", 1, 0, 6, true );
UPAddressMapField( "WI",     1, 6, 1 );
UPAddressMapField( "RI",     1, 7, 1 );
UPAddressMapField( "",       1, 8, 2, true );
UPAddressMapField( "CW",     1, 10, 1 );
UPAddressMapField( "CR",     1, 11, 1 );
UPAddressMapField( "WE",     1, 12, 1 );
UPAddressMapField( "RE",     1, 13, 1 );

UPAddressMapField( "WSLC",   2, 0, 2 );
UPAddressMapField( "WSRC",   2, 2, 2 );
UPAddressMapField( "RALC",   2, 4, 4 );
UPAddressMapField( "RARC",   2, 8, 6 );

UPAddressMapField( "Left data",  3, 0, 14 );
UPAddressMapField( "Right data", 4, 0, 14 );

UPAddressMapDiagram( 4, 14 );

UPAddressMapLeftLabel( "0xFF203040", 1 );
UPAddressMapLeftLabel( "0xFF203044", 2 );
UPAddressMapLeftLabel( "0xFF203048", 3 );
UPAddressMapLeftLabel( "0xFF20304C", 4 );

UPAddressMapRightLabel( "Control",   1 );
UPAddressMapRightLabel( "Fifospace", 2 );
UPAddressMapRightLabel( "Leftdata",  3 );
UPAddressMapRightLabel( "Rightdata", 4 );

