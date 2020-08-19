<img src="logo.jpg" width="40%">  


# LGCA FHP

A FHP lattice-gas cellular automata with understandable visualisation and an easily customisable input file.

## Usage

### Start a simulation
```bash
./build/bin/latticeGas_FHP <nameOfInputfile>.xml
```

### Visualise simulation results
```bash
python3 visualize_results.py <pathToCSV> <type> <video> <skip plotted files>
```

##### Example tunnel flow:
```bash
./build/bin/latticeGas_FHP tests/tunnel_flow/tunnel_flow.xml
```

```bash
python3 visualize_results.py tests/tunnel_flow/ all no yes
```

## Compiling
```bash
make build
```
or
```bash
make debug
```
