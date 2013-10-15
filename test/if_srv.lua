#!../out/lua

tcpsock = net.Socket('TCP')
ip      = net.IpEndpoint('10.128.3.131', 8888)

tcpsock:bind(ip)
tcpsock:listen(5)

conns  = { tcpsock }

while true do
	res = net.select(conns, {})
	io.write('LOOP['..#res..']:')
	for n,cli in ipairs(res) do
		io.write('  '..tostring(cli)..'')
	end
	print()

	for n,cli in ipairs(res) do
		if cli == tcpsock then
			local s,a =  tcpsock:accept()
			table.insert(conns, s)
			print('\tCONNECT: ' ..tostring(s).. " FROM:  "..tostring(a) )
		else
			msg, len = cli:recv()
			if len<1 then
				for i,v in ipairs(conns) do
					if v == cli then
						table.remove(conns, i)
						print('\tCLOSED: '.. tostring(v))
					end
				end
			else
				print('\tRECIEVED['..len..']: '..msg)
			end
		end
	end
end
tcpsock:close()
