
## Software for my hardware winch, used for reinforcement learning ##

Find more details [here](https://nimblemetal.com/2018/07/23/a-winch-for-reinforcement-learning/)

### Reinforcement Learning Pain When Training a Real Robot

Reinforcement learning often takes thousands, or hundreds of thousands of episodes before the neural network is trained well enough to accomplish a task. We are getting better at methods to reduce the number of episodes, but for now its still a laborious process. When working with a real robot (vs a simulation), the robot must be reset in between each episode. This usually involves picking up a fallen robot, or moving a robot back to a specific position, so it can try again. Even if we put aside the damage that can happen to an untrained robot during these episodes, the process of repeatedly setting up the robot for each episode is tremendously time consuming.

### The Fantasy

The fantasy solution is that I'm off on vacation while my robot is training itself, 24/7, without any need of a human to help it. I figured it was worth trying to make this fantasy come true. So here is that adventure.

### The Winch

I built a winch for Beaker, my two-wheeled balancing robot. The winch had two main jobs, equally important:

0. Protect the balancing robot so when it falls, it does not fall all the way down.
0. Reset the robot for the next episode.

