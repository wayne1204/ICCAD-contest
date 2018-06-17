import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.patches import Rectangle
import os
import sys

class CircuitParser():
    def __init__(self):
        self.layers = [[] for item in range(9)]
        self.layers_cnet = [[] for item in range(9)]
        self.boundary = []
        self.cnet = set()
        self.design = ''
        fig, self.ax = plt.subplots(1)
    
    def parseCnet(self, fileName):
        text = open(fileName, 'r')
        for i, line in enumerate(text):
            if i == 0:
                directory, fname = os.path.split(fileName)
                self.design = os.path.join(directory, line.split()[1])
                print('design name: ', self.design)
            elif i == 4:
                row = line.split()[1:]
                num = [int(i) for i in row]
                self.cnet = set(num)

    def parseDesign(self): 
        text = open(self.design, 'r')
        for i, line in enumerate(text):
            if i == 0:
                row = line.replace(';', ' ').split(' ')
                self.boundary = [int(row[i])//1000 for i in range(4)]
                print('bounary (divided by 1000): ', self.boundary)
            else:
                row = line.split()
                idx = int(row[6]) - 1 
                a = [int(row[i]) for i in range(1, 5)]
                if int(row[5]) in self.cnet:
                    self.layers_cnet[idx].append(a)
                else:
                    self.layers[idx].append(a)

    def insert_polygon(self, points, color='black'):
        errorboxes = []
        rect = Rectangle((points[0], points[1]), points[2] - points[0],
                        points[3] - points[1])
        errorboxes.append(rect)
        pc = PatchCollection(errorboxes, facecolor=color, alpha=0.5,
                             edgecolor=color)
        self.ax.add_collection(pc)
        artists = self.ax.errorbar(0, 0, xerr=0, yerr=0, fmt='None', ecolor='k')
    
    def scaling(self, num):
        for j in range(len(self.layers[num])):
            self.layers[num][j] = [i/1000.0 for i in self.layers[num][j]]
        for j in range(len(self.layers_cnet[num])):
            self.layers_cnet[num][j] = [i/1000.0 for i in self.layers_cnet[num][j]]
        print('scaling layer #{}...'.format(num+1))

    
    def adjustPoly(self, poly, lb_x, lb_y, rt_x, rt_y):
        ret = poly[:]
        if poly[0] > rt_x or poly[2] < lb_x:
            return []
        if poly[1] > rt_y or poly[3] < lb_y:
            return []

        if poly[0] < lb_x:
            ret[0] = lb_x
        if poly[1] < lb_y:
            ret[1] = lb_y
        if poly[2] > rt_x:
            ret[2] = rt_x
        if poly[3] > rt_y:
            ret[3] = rt_y
        return ret

    def plot(self, num):
        WINDOW = 30
        HEIGHT = (self.boundary[3] - self.boundary[1]) // WINDOW
        WIDTH  = (self.boundary[2] - self.boundary[0]) // WINDOW
        for i in range(HEIGHT):
            for j in range(WIDTH):
                fig, self.ax = plt.subplots(1)
                lb_x = self.boundary[0] + j * WINDOW
                lb_y = self.boundary[1] + i * WINDOW
                rt_x = self.boundary[0] + (j+1) * WINDOW
                rt_y = self.boundary[1] + (i+1) * WINDOW
                print('window #{} / {}'.format(i * WIDTH + j+1, WIDTH * HEIGHT), end='\r')

                for k in range(len(self.layers[num])):
                    insert = self.adjustPoly(self.layers[num][k], lb_x, lb_y, rt_x, rt_y)
                    if insert != []:
                        self.insert_polygon(insert, 'black')

                for k in range(len(self.layers_cnet[num])):
                    insert = self.adjustPoly(self.layers_cnet[num][k], lb_x, lb_y, rt_x, rt_y)
                    if insert != []:
                        self.insert_polygon(insert, 'red')

                plt.axis([lb_x, rt_x, lb_y, rt_y])
                plt.title('Layer_{}_{}_{}'.format(num+1, i, j))
                plt.savefig("visualize/layer{}/layer_{}_{}_{}.png".format(num+1,num+1, i, j))
                plt.close()
        print()

def main():
    cp = CircuitParser()
    cp.parseCnet(sys.argv[1])
    cp.parseDesign()  

    for i in range(9):
        path = os.path.join('visualize', 'layer{}'.format(i+1))
        os.mkdir(path)
        cp.scaling(i)
        cp.plot(i)
        

if __name__ == "__main__":
    main()