import os
import sys
import types
import subprocess
import re
import atexit

# subprocess wrapper that uses shell on Windows

class Popen(subprocess.Popen):
    def __init__(self, args, bufsize=0, executable=None,
                 stdin=None, stdout=None, stderr=None,
                 preexec_fn=None, close_fds=False, shell=None,
                 cwd=None, env=None, universal_newlines=False,
                 startupinfo=None, creationflags=0):

	if shell == None:
	    shell = (sys.platform == "win32")

	subprocess.Popen.__init__(self, args, bufsize, executable,
                 stdin, stdout, stderr,
                 preexec_fn, close_fds, shell,
                 cwd, env, universal_newlines,
                 startupinfo, creationflags)

PIPE = subprocess.PIPE
STDOUT = subprocess.STDOUT

def call(*args, **kwargs):
    return Popen(*args, **kwargs).wait()

# GRASS-oriented interface to subprocess module

_popen_args = ["bufsize", "executable", "stdin", "stdout", "stderr",
	       "preexec_fn", "close_fds", "cwd", "env",
	       "universal_newlines", "startupinfo", "creationflags"]

def _make_val(val):
    if isinstance(val, types.StringType):
	return val
    if isinstance(val, types.ListType):
	return ",".join(map(_make_val, val))
    if isinstance(val, types.TupleType):
	return _make_val(list(val))
    return str(val)

def make_command(prog, flags = "", overwrite = False, quiet = False, verbose = False, **options):
    args = [prog]
    if overwrite:
	args.append("--o")
    if quiet:
	args.append("--q")
    if verbose:
	args.append("--v")
    if flags:
	args.append("-%s" % flags)
    for opt, val in options.iteritems():
	if val != None:
	    if opt[0] == '_':
		opt = opt[1:]
	    args.append("%s=%s" % (opt, _make_val(val)))
    return args

def start_command(prog, flags = "", overwrite = False, quiet = False, verbose = False, **kwargs):
    options = {}
    popts = {}
    for opt, val in kwargs.iteritems():
	if opt in _popen_args:
	    popts[opt] = val
	else:
	    options[opt] = val
    args = make_command(prog, flags, overwrite, quiet, verbose, **options)
    return Popen(args, **popts)

def run_command(*args, **kwargs):
    ps = start_command(*args, **kwargs)
    return ps.wait()

def pipe_command(*args, **kwargs):
    kwargs['stdout'] = PIPE
    return start_command(*args, **kwargs)

def feed_command(*args, **kwargs):
    kwargs['stdin'] = PIPE
    return start_command(*args, **kwargs)

def read_command(*args, **kwargs):
    ps = pipe_command(*args, **kwargs)
    return ps.communicate()[0]

def write_command(*args, **kwargs):
    stdin = kwargs['stdin']
    kwargs['stdin'] = PIPE
    p = start_command(*args, **kwargs)
    p.stdin.write(stdin)
    p.stdin.close()
    return p.wait()

def exec_command(prog, flags = "", overwrite = False, quiet = False, verbose = False, env = None, **kwargs):
    args = make_command(prog, flags, overwrite, quiet, verbose, **kwargs)
    if env == None:
	env = os.environ
    os.execvpe(prog, args, env)

# interface to g.message

def message(msg, flag = None):
    run_command("g.message", flags = flag, message = msg)

def debug(msg):
    message(msg, flag = 'd')

def verbose(msg):
    message(msg, flag = 'v')

def info(msg):
    message(msg, flag = 'i')

def warning(msg):
    message(msg, flag = 'w')

def error(msg):
    message(msg, flag = 'e')

def fatal(msg):
    error(msg)
    sys.exit(1)

# interface to g.parser

def _parse_env():
    options = {}
    flags = {}
    for var, val in os.environ.iteritems():
	if var.startswith("GIS_OPT_"):
	    opt = var.replace("GIS_OPT_", "", 1).lower()
	    options[opt] = val;
	if var.startswith("GIS_FLAG_"):
	    flg = var.replace("GIS_FLAG_", "", 1).lower()
	    flags[flg] = bool(int(val));
    return (options, flags)

def parser():
    if not os.getenv("GISBASE"):
        print >> sys.stderr, "You must be in GRASS GIS to run this program."
        sys.exit(1)

    if len(sys.argv) > 1 and sys.argv[1] == "@ARGS_PARSED@":
	return _parse_env()

    cmdline = [basename(sys.argv[0])]
    cmdline += ['"' + arg + '"' for arg in sys.argv[1:]]
    os.environ['CMDLINE'] = ' '.join(cmdline)

    argv = sys.argv[:]
    name = argv[0]
    if not os.path.isabs(name):
	if os.sep in name or (os.altsep and os.altsep in name):
	    argv[0] = os.path.abspath(name)
	else:
	    argv[0] = os.path.join(sys.path[0], name)

    os.execvp("g.parser", [name] + argv)
    raise OSError("error executing g.parser")

# interface to g.tempfile

def tempfile():
    return read_command("g.tempfile", pid = os.getpid()).strip()

# key-value parsers

def parse_key_val(s, sep = '=', dflt = None):
    result = {}
    for line in s.splitlines():
	kv = line.split(sep, 1)
	k = kv[0].strip()
	if len(kv) > 1:
	    v = kv[1]
	else:
	    v = dflt
	result[k] = v
    return result

_kv_regex = None

def parse_key_val2(s):
    global _kv_regex
    if _kv_regex == None:
	_kv_regex = re.compile("([^=]+)='(.*)';?")
    result = []
    for line in s.splitlines():
	m = _kv_regex.match(line)
	if m != None:
	    result.append(m.groups())
	else:
	    result.append(line.split('=', 1))
    return dict(result)

# interface to g.gisenv

def gisenv():
    return parse_key_val2(read_command("g.gisenv"))

# interface to g.region

def region():
    s = read_command("g.region", flags='g')
    return parse_key_val(s)

def use_temp_region():
    name = "tmp.%s.%d" % (os.path.basename(sys.argv[0]), os.getpid())
    run_command("g.region", save = name)
    os.environ['WIND_OVERRIDE'] = name
    atexit.register(del_temp_region)

def del_temp_region():
    try:
	name = os.environ.pop('WIND_OVERRIDE')
	run_command("g.remove", quiet = True, region = name)
    except:
	pass

# interface to g.findfile

def find_file(name, element = 'cell', mapset = None):
    s = read_command("g.findfile", element = element, file = name, mapset = mapset)
    return parse_key_val2(s)

# interface to g.list

def list_grouped(type):
    dashes_re = re.compile("^----+$")
    mapset_re = re.compile("<(.*)>:$")
    result = {}
    mapset = None
    for line in read_command("g.list", type = type).splitlines():
	if line == "":
	    continue
	if dashes_re.match(line):
	    continue
	m = mapset_re.search(line)
	if m:
	    mapset = m.group(1)
	    result[mapset] = []
	    continue
        if mapset:
            result[mapset].extend(line.split())
    return result

def _concat(xs):
    result = []
    for x in xs:
	result.extend(x)
    return result

def list_pairs(type):
    return _concat([[(map, mapset) for map in maps]
		    for mapset, maps in list_grouped(type).iteritems()])

def list_strings(type):
    return ["%s@%s" % pair for pair in list_pairs(type)]

# color parsing

named_colors = {
    "white":   (1.00, 1.00, 1.00),
    "black":   (0.00, 0.00, 0.00),
    "red":     (1.00, 0.00, 0.00),
    "green":   (0.00, 1.00, 0.00),
    "blue":    (0.00, 0.00, 1.00),
    "yellow":  (1.00, 1.00, 0.00),
    "magenta": (1.00, 0.00, 1.00),
    "cyan":    (0.00, 1.00, 1.00),
    "aqua":    (0.00, 0.75, 0.75),
    "grey":    (0.75, 0.75, 0.75),
    "gray":    (0.75, 0.75, 0.75),
    "orange":  (1.00, 0.50, 0.00),
    "brown":   (0.75, 0.50, 0.25),
    "purple":  (0.50, 0.00, 1.00),
    "violet":  (0.50, 0.00, 1.00),
    "indigo":  (0.00, 0.50, 1.00)}

def parse_color(val, dflt = None):
    if val in named_colors:
        return named_colors[val]

    vals = val.split(':')
    if len(vals) == 3:
        return tuple(float(v) / 255 for v in vals)

    return dflt

# check GRASS_OVERWRITE

def overwrite():
    owstr = 'GRASS_OVERWRITE'
    return owstr in os.environ and os.environ[owstr] != '0'

# check GRASS_VERBOSE

def verbosity():
    vbstr = 'GRASS_VERBOSE'
    if vbstr:
	return int(vbstr)
    else:
	return 0

## various utilities, not specific to GRASS

# basename inc. extension stripping

def basename(path, ext = None):
    name = os.path.basename(path)
    if not ext:
	return name
    fs = name.rsplit('.', 1)
    if len(fs) > 1 and fs[1].lower() == ext:
	name = fs[0]
    return name

# find a program (replacement for "which")

def find_program(pgm, args = []):
    nuldev = file(os.devnull, 'w+')
    try:
	call([pgm] + args, stdin = nuldev, stdout = nuldev, stderr = nuldev)
	found = True
    except:
	found = False
    nuldev.close()
    return found

# try to remove a file, without complaints

def try_remove(path):
    try:
	os.remove(path)
    except:
	pass

# try to remove a directory, without complaints

def try_rmdir(path):
    try:
	os.rmdir(path)
    except:
	pass

# run "v.db.connect -g ..." and parse output

def vector_db(map, layer = None, **args):
    s = read_command('v.db.connect', flags = 'g', map = map, layer = layer, **args)
    result = []
    for l in s.splitlines():
	f = l.split(' ')
	if len(f) != 5:
	    continue
	if layer and int(layer) == int(f[0]):
	    return f
	result.append(f)
    if not layer:
	return result

# run "db.describe -c ..." and parse output

def db_describe(table, **args):
    s = read_command('db.describe', flags = 'c', table = table, **args)
    if not s:
	return None
    cols = []
    result = {}
    for l in s.splitlines():
	f = l.split(':')
	key = f[0]
	f[1] = f[1].lstrip(' ')
	if key.startswith('Column '):
	    n = int(key.split(' ')[1])
	    cols.insert(n, f[1:])
	elif key in ['ncols', 'nrows']:
	    result[key] = int(f[1])
	else:
	    result[key] = f[1:]
    result['cols'] = cols
    return result

# run "db.connect -p" and parse output

def db_connection():
    s = read_command('db.connect', flags = 'p')
    return parse_key_val(s, sep = ':')

# run "v.info -c ..." and parse output

def vector_columns(map, layer = None, **args):
    s = read_command('v.info', flags = 'c', map = map, layer = layer, quiet = True, **args)
    result = []
    for line in s.splitlines():
	f = line.split('|')
	if len(f) == 2:
	    result.append(f)
    return result

# add vector history

def vector_history(map):
    run_command('v.support', map = map, cmdhist = os.environ['CMDLINE'])

# add raster history

def raster_history(map):
    run_command('r.support', map = map, history = os.environ['CMDLINE'])

