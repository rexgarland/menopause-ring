import numpy as np
from glumpy import app, gloo, gl
import serial
import os

# ------------- SERIAL HELPERS -------------

def get_serial_sample(ser):
	"""Parse serial line as csv"""
	found = False
	while not found:
		try:
			line = ser.readline()
			line = line.strip().decode('UTF-8')
		except:
			print("Could not decode line '{}'".format(line))
			continue
		if ((not line) or (line[0]=='#')):
			print("line '{}' ignored".format(line))
			continue
		try:
			out_dict = parse_line(line)
			found = True
		except:
			print("Error")
	return out_dict

def get_usb_port():
	"""Find arduino"""
	usb_ports = [a for a in os.listdir('/dev') if 'usb' in a]
	assert usb_ports, "Could not find serial port for arduino."
	port = os.path.join('/dev',usb_ports[0])
	print("Found serial port '{}'".format(port))
	return port

class SerialMonitor(serial.Serial):
	def __init__(self):
		self.open_serial_port()

	def __enter__(self):
		return self

	def open_serial_port(self):
		super().__init__()
		self.port = get_usb_port()
		self.baudrate = 115200
		self.timeout = 10
		self.open()
		assert self.is_open, "Could not open serial point {}".format(self.port)
		self.flush()
		self.readline()

	def sample(self):
		return get_serial_sample(self)

	def __exit__(self, exc_type, exc_value, traceback):
		self.close()

# ------------- GLUMPY SECTION -------------

vertex = """
uniform int index, size, num_plots;
attribute float x_index, y_index, y_value;
varying float do_discard;
void main (void)
{
	float x = 2*(mod(x_index - index, size) / (size)) - 1.0;
	if ((x >= +1.0) || (x <= -1.0)) do_discard = 1;
	else                            do_discard = 0;
	float y = (2*((y_index+.5)/(num_plots))-1) + y_value;
	gl_Position = vec4(x, y, 0, 1);
}
"""

fragment = """
varying float do_discard;
void main(void)
{
	if (do_discard > 0) discard;
	gl_FragColor = vec4(0,0,0,1);
}
"""

vertex_dot = """
attribute vec2 position;
attribute float enable;
varying float v_enable;
void main (void) {
	v_enable = enable;
	gl_Position = vec4(position, 0.0, 1.0);
	gl_PointSize = 10;
}
"""

fragment_dot = """
varying float v_enable;
void main (void) {
	if (v_enable == 0) discard;
	gl_FragColor = vec4(1, 0, 0, 1);
}
"""

window = app.Window(width=1200, height=1000, color=(1,1,1,1))

num_plots, size = 4, 800
program = gloo.Program(vertex, fragment, count=size*num_plots)
program["size"] = size
program["num_plots"] = num_plots
program["x_index"] = np.repeat(np.arange(size),num_plots)
program["y_index"] = np.tile(np.arange(num_plots),size)
program["y_value"] = 0

num_beats = 30
data = np.zeros(num_beats, dtype=[('position',    np.float32, 2),
                          ('enable',      np.float32, 1)])
data = data.view(gloo.VertexBuffer)

program_dot = gloo.Program(vertex_dot, fragment_dot, count=1)
program_dot.bind(data)

# Compute indices
I = np.arange(num_plots * size, dtype=np.uint32).reshape(size, -1).T
I = np.roll(np.repeat(I, 2, axis=1), -1, axis=1)
I = I.view(gloo.IndexBuffer)

# ------------- APP SECTION -------------

with SerialMonitor() as monitor:

	latest_bpm = 0

	@window.event
	def on_draw(dt):
		# global size, num_plots
		global latest_bpm
		window.clear()
		program.draw(gl.GL_LINES, I)
		program_dot.draw(gl.GL_POINTS)
		index = int(program["index"])
		y = program["y_value"].reshape(size,num_plots)
		
		beats = []
		minYs = []
		bpms = []
		hr = []
		prior = []
		posterior = []
		for _ in range(7):
			out = monitor.sample()
			hr.append(out['y'])
			prior.append(out['prior'])
			posterior.append(out['posterior'])
			if 'beat' in out:
				beats.append(out['beat'])
				minYs.append(out['minY'])
				bpms.append(out['bpm'])
		calmo_state['index'] = out['sampleCount']
		hr_val = np.min(hr)/100
		prior_val = np.average(prior)/2
		posterior_val = np.average(posterior)/200

		yscale = 1.0/num_plots
		y[index] = yscale * np.array([latest_bpm/100, posterior_val, prior_val, hr_val])

		program["index"] = (index + 1) % size
		display_state['index'] += 1

		for i in range(len(beats)):
			beat = beats[i]
			minY = minYs[i]
			latest_bpm = bpms[-1]
			# update beat positions
			program_dot["enable"][display_state['current_beat']] = True
			program_dot["position"][display_state['current_beat']] = 1-2*(float(calmo_state['index']-beat)/7/size), 2*(float(num_plots-1+0.5)/num_plots)-1+minY/100*yscale

			# cycle current beat
			display_state['current_beat'] = (display_state['current_beat']+1) % num_beats

		program_dot["position"][:,0] -= 2.0/size

	offset = monitor.sample()['sampleCount']
	calmo_state = {'index':0, 'beats':[]}
	display_state = {'index':size, 'beat_positions': [size]*num_beats, 'current_beat':0}

	app.run()
