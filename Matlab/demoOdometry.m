% Drive Forwards
robot.sendPacket('SWV 100 100')
figure
t = []; l = [];
h = plot(t, l);
while true
	[str, time] = robot.sendPacket('GOM')
	d = sscanf(str, '%f');
	l = [l, d(2)];
	t = [t, time];
	
	% Update Plot
	plot(t,l)
	
	drawnow
	pause(0.1)
end