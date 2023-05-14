#!/usr/bin/python3
# -*- encoding: utf-8 -*-
import re,sys
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np

class animator:
    def __init__(self):
        self.step_begin_pat = re.compile("step: (?P<step>\d+) t: (?P<t>.*?) T: (?P<T>.*?) n: (?P<n>\d+) U: (?P<U>.*?) K: (?P<K>.*?) U\+K: (?P<UK>.*)")
        self.particle_pat = re.compile("(?P<idx>\d+) m: (?P<m>.*?) pos: (?P<x>.*?) (?P<y>.*?) (?P<z>.*?) vel: (?P<vx>.*?) (?P<vy>.*?) (?P<vz>.*?) acc: (?P<ax>.*?) (?P<ay>.*?) (?P<az>.*)")
        # put particle positions
        self.positions = []
        self.fig = plt.figure()
        #self.ax = self.fig.add_subplot(111, projection='3d')
        self.ax0 = self.fig.add_subplot(211, projection='3d')
        self.ax1 = self.fig.add_subplot(212)
        self.scatter = None
        self.ts = []
        self.UKs = []
        self.Us = []
        self.Ks = []

    def show(self):
        if len(self.positions) > 0:
            a = np.array(self.positions)
            self.positions = []
            if self.scatter is None:
                self.scatter = self.ax0.scatter(a[:,0], a[:,1], a[:,2])
                [ u ] = self.Us
                self.UK_plot, = self.ax1.plot(self.ts, self.UKs)
                self.U_plot, = self.ax1.plot(self.ts, self.Us)
                self.K_plot, = self.ax1.plot(self.ts, self.Ks)
            else:
                self.scatter._offsets3d = (a[:,0], a[:,1], a[:,2])
                #self.scatter._offsets3d = (a[:,0], a[:,1])
                self.UK_plot.set_data(self.ts, self.UKs)
                self.U_plot.set_data(self.ts, self.Us)
                self.K_plot.set_data(self.ts, self.Ks)
                ax = self.ax1
                t0,t1 = ax.get_xlim()
                y0,y1 = ax.get_ylim()
                yc = (y0 + y1) / 2
                if max(self.UKs[-1], self.Us[-1], self.Ks[-1]) > y1 \
                   or min(self.UKs[-1], self.Us[-1], self.Ks[-1]) < y0:
                    y0 = y0 - (y1 - y0)
                    y1 = y1 + (y1 - y0)
                    self.ax1.set_ylim([ y0, y1 ])
                if self.ts[-1] > t1:
                    t1 = 2 * t1
                    self.ax1.set_xlim([ 0, t1 ])
            return [ self.scatter, self.UK_plot, self.U_plot, self.K_plot ]
        else:
            return None
        
    def step_begin(self, m):
        s = self.show()
        g = m.group
        self.step = int(g("step"))
        self.t = float(g("t"))
        self.T = float(g("T"))
        self.n = int(g("n"))
        self.U = float(g("U"))
        self.K = float(g("K"))
        self.UK = float(g("UK"))
        self.ts.append(self.t)
        self.UKs.append(self.UK)
        self.Us.append(self.U)
        self.Ks.append(self.K)
        # print(self.t)
        return s

    def particle(self, m):
        g = m.group
        self.idx = int(g("idx"))
        if self.idx % 1 == 0:
            self.m = float(g("m"))
            self.pos = (float(g("x")), float(g("y")), float(g("z")))
            self.vel = (float(g("vx")), float(g("vy")), float(g("vz")))
            self.acc = (float(g("ax")), float(g("ay")), float(g("az")))
            self.positions.append(self.pos)
        
    def animate_file(self, filename):
        if filename == "-":
            fp = sys.stdin
        else:
            fp = open(filename)
        for line in fp:
            m = self.step_begin_pat.match(line)
            if m:
                artists = self.step_begin(m)
                if artists is not None:
                    yield artists
            m = self.particle_pat.match(line)
            if m:
                if self.step % 10 == 0:
                    self.particle(m)
        fp.close()

# おまじない(コピペして下さい)
def do_animation(iterator, **kwargs):
    def anime_fun(*args):
        try:
            return next(iterator)
        except StopIteration:
            return []
    ani = animation.FuncAnimation(plt.gcf(), anime_fun, **kwargs)
    #ani.save("anim.mp4")
    plt.show()
                
def main():
    dat = sys.argv[1]
    an = animator()
    g = an.animate_file(dat)
    do_animation(g)
        
main()
