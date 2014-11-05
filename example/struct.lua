xt=require'xt'


p = xt.Pack.IntB( 2 )
c = xt.Pack.Int( 2 )

ts = xt.Pack.Struct (
	p,
	xt.Pack.Int( 2 ),
	xt.Pack.Int( 1 ),
	xt.Pack.Int( 1 )
)

ss = xt.Pack.Struct (
	p,
	p,
	xt.Pack.Struct (
		xt.Pack.Bit( 1 ),  -- is the train set healthy?
		xt.Pack.Bit( 1 ),  -- is the train set stationary?
		xt.Pack.Bit( 1 ),  -- is the train set in maint mode?
		xt.Pack.Bit( 1 ),  -- is a passenger request active?
		xt.Pack.Bit( 1 ),  -- is a file waiting for download (RTDM ready)
		xt.Pack.Bit( 1 ),  -- is the connection to the vmds lost?
		xt.Pack.Bit( 1 ),  -- is the train set in shop mode?
		xt.Pack.Bit( 1 )   -- padding
	),
	xt.Pack.Int(1),
	xt.Pack.Int(1)
)


s = xt.Pack.Struct (
	{ length       = xt.Pack.Int( 2 ) },
	{ ['type']     = xt.Pack.Int( 2 ) },
 	{ ['@status']  = xt.Pack.Int( 1 ) },
 	{ ConsistCount = xt.Pack.Int( 1 ) }
)


t = xt.Pack.Struct (
	{ length   = xt.Pack.Int( 2 ) },
	{ ['type'] = xt.Pack.Int( 2 ) },
	-- 	-- BitField is a special type as the constructor resets the actual offset for each single bit
-- 	-- boolean flags -> status
	{ status   = xt.Pack.Struct (
		{isHealthy   = xt.Pack.Bit( 1 )},  -- is the train set healthy?
		{isZeroSpeed = xt.Pack.Bit( 1 )},  -- is the train set stationary?
		{isMaintMode = xt.Pack.Bit( 1 )},  -- is the train set in maint mode?
		{isPassReq   = xt.Pack.Bit( 1 )},  -- is a passenger request active?
		{isFileForDl = xt.Pack.Bit( 1 )},  -- is a file waiting for download (RTDM ready)
		{isVmdsConnd = xt.Pack.Bit( 1 )},  -- is the connection to the vmds lost?
		{isShopMode  = xt.Pack.Bit( 1 )},  -- is the train set in shop mode?
		{padding     = xt.Pack.Bit( 1 )}   -- padding
	)},
	{internal       = ss},
 	{['@status']    = xt.Pack.Int( 1 )},
	xt.Pack.Int( 2 ),
 	{ConsistCount   = xt.Pack.Int( 1 )}
)

b=xt.Buffer( 'ABCDEFGH' )

--for k,v in pairs(s) do print( k,v ) end




a = xt.Pack.Array( xt.Pack.IntB(2), 4 )
