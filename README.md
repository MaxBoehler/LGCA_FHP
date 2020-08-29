<img src="logo.jpg" width="40%">  


# LGCA FHP

A FHP lattice-gas cellular automata with understandable visualisation and an easily customisable input file.

## Usage

### Start a simulation
```bash
mpiexec -n <xMesh*yMesh> ./build/bin/latticeGas_FHP <nameOfInputfile>.xml
```

### Visualise simulation results
```bash
python3 visualize_results.py <pathToCSV> <type> <video>
```

##### Example tunnel flow:
```bash
mpiexec -n 4 ./build/bin/latticeGas_FHP tests/tunnel_flow/tunnel_flow.xml
```

```bash
python3 visualize_results.py tests/tunnel_flow/ all no
```

## Compiling
```bash
make build
```
or
```bash
make debug
```
