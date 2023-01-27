import subprocess
import random
import itertools
DEBUG = False

INITIAL_VALUES = [1, 2] # 1: distinct colors, 2: greedy
# CWT_CANDIDATES includes all feasible combinations of 0/10 weights for 6 color reordering strategies
CWT_CANDIDATES = list(itertools.product([0, 10], repeat=6))
CWT_CANDIDATES.remove((0, 0, 0, 0, 0, 0))
# VWT_CANDIDATES includes all feasible combinations of 0/10 weights for 4 vertex reordering strategies
VWT_CANDIDATES = list(itertools.product([0, 10], repeat=4))
VWT_CANDIDATES.remove((0, 0, 0, 0))

class SIGConfig:
    def __init__(self, **kwargs):
        self.input = 'input_01.txt'
        self.INITIAL = 1
        self.MAXITER = 10
        self.TARGET = 0
        self.CWEIGHTS = [10, 10, 10, 10, 10, 10]
        self.VWEIGHTS = [10, 10, 10, 10]
        self.RLIMIT = 10
        self.TRIAL = 1
        self.SEED = None
        self.__dict__.update(kwargs)
    
    def write_params(self, new_seed=True):
        with open('params.txt', 'w') as f:
            f.write(f'inp input_01.txt\n')
            f.write(f'int {self.INITIAL}\n')
            f.write(f'max {self.MAXITER}\n')
            f.write(f'tar {self.TARGET}\n')
            f.write(f'cwt {" ".join([str(i) for i in self.CWEIGHTS])}\n')
            f.write(f'vwt {" ".join([str(i) for i in self.VWEIGHTS])}\n')
            f.write(f'rev {self.RLIMIT}\n')
            f.write(f'tri {self.TRIAL}\n')
            if new_seed:
                self.SEED = random.getrandbits(63)
            if self.SEED:
                f.write(f'see {self.SEED}\n')

def generate_input(n=30, m=300):
    if m > n*(n-1)/2:
        raise ValueError('Too many edges')
    subprocess.run(f'./randomgraph {n} {m} {random.getrandbits(63)} > input_01.txt', shell=True, check=True, capture_output=True)

def run_sig():
    output=subprocess.run('./sig <params.txt', shell=True, check=True, capture_output=True).stdout.decode('utf-8')
    if DEBUG:
        print(output)
    results=output.split('\n')[-2].split()
    return results[-2:]

# Test SIG with fixed parameters
# random_input: each run, a new random input graph is generated
# random_seed: each run, a new random seed is used by SIG
def repeat_test(config=SIGConfig(), repeat=10, random_input=True, n=30, m=300, random_seed=True):
    color_count_list = []
    color_score_list = []
    config.write_params()
    for i in range(repeat):
        if random_input:
            generate_input(n, m)
        if random_seed:
            config.write_params(new_seed=True)
        color_count, color_score = run_sig()
        color_count_list.append(int(color_count))
        color_score_list.append(int(color_score))
    if DEBUG:
        print(color_count_list, color_score_list)
    return color_count_list, color_score_list

# Test SIG with one varying parameter and others fixed
def test_with_INITIAL_varying():
    # Set the default parameters here
    config = SIGConfig(INITIAL=1, MAXITER=10, TARGET=0, CWEIGHTS=[10, 10, 10, 10, 10, 10], VWEIGHTS=[10, 10, 10, 10], RLIMIT=10, TRIAL=1, SEED=None)
    # The size of input graphs
    n = 30 # number of vertices
    m = 300 # number of edges
    # Specify the candidates for parameter value here
    for i in INITIAL_VALUES:
        config.INITIAL = i
        config.write_params()
        # Specify the number of runs here
        color_count_list, color_score_list = repeat_test(config, repeat=500, random_input=True, n=n, m=m, random_seed=True)
        # Display average color_count and color_score
        print(f'INITIAL={i}:', sum(color_count_list)/len(color_count_list), sum(color_score_list)/len(color_score_list))

if __name__ == '__main__':
    # generate_input()
    # config = SIGConfig()
    # config.write_params()
    # color_count, color_score = run_sig()
    # print(color_count, color_score)
    # color_count_list, color_score_list = repeat_test(repeat=100)
    test_with_INITIAL_varying()
