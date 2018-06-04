/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
	
  num_particles = 150;

  normal_distribution<double> dist_x_init(x, std[0]);
  normal_distribution<double> dist_y_init(y, std[1]);
  normal_distribution<double> dist_theta_init(theta, std[2]);

  for(int i = 0; i < num_particles; i++) {

    Particle p;
    p.id = i;
    p.x = dist_x_init(gen);
    p.y = dist_y_init(gen);
    p.theta = dist_theta_init(gen);
    p.weight = 1.0;

    particles.push_back(p);
    weights.push_back(p.weight);
  }

  is_initialized = true;

}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
  // TODO: Add measurements to each particle and add random Gaussian noise.
  // NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
  //  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
  //  http://www.cplusplus.com/reference/random/default_random_engine/

  normal_distribution<double> dist_x(0, std_pos[0]);
  normal_distribution<double> dist_y(0, std_pos[1]);
  normal_distribution<double> dist_theta(0, std_pos[2]);

  for(int i = 0; i < num_particles; i++) {

    double x = particles[i].x;
    double y = particles[i].y;
    double theta = particles[i].theta;

    if(fabs(yaw_rate) > 0.0001) {

      double yaw_rate_dot = yaw_rate * delta_t;
      double v_over_yaw = velocity / yaw_rate;

      x += v_over_yaw * (sin(theta + yaw_rate_dot) - sin(theta));
      y += v_over_yaw * (cos(theta) - cos(theta + yaw_rate_dot));
      theta += yaw_rate_dot;
      
    } else {

      double velocity_dot = velocity * delta_t;

      x += velocity_dot * cos(particles[i].theta);
      y += velocity_dot * sin(particles[i].theta);

    }

    x += dist_x(gen);
    y += dist_y(gen);
    theta += dist_theta(gen);

    particles[i].x = x;
    particles[i].y = y;
    particles[i].theta = theta;
  }

}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
  for(int i = 0; i < observations.size(); i++) {

    LandmarkObs obs = observations[i];

    double min_dst = numeric_limits<double>::max();

    int map_id = -1;

    for(int j = 0; j < predicted.size(); j++) {
      LandmarkObs pred = predicted[j];

      double obs_pred_dst = dist(obs.x, obs.y, pred.x, pred.y);

      if(obs_pred_dst < min_dst) {
        min_dst = obs_pred_dst;
        map_id = pred.id;
      }
    }

    observations[i].id = map_id;
  }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
	
  double weight_norm = 0.0;
	
  for(int i = 0; i < num_particles; i++) {

    double par_x = particles[i].x;
    double par_y = particles[i].y;
    double par_theta = particles[i].theta;


    // Transorm observations from vehicle to map coordinates
    vector<LandmarkObs> t_obs_list;

    for(int j = 0; j < observations.size(); j++) {

      LandmarkObs t_obs;
      t_obs.id = j;
      t_obs.x = par_x + (cos(par_theta) * observations[j].x) -
        (sin(par_theta) * observations[j].y);
      t_obs.y = par_y + (sin(par_theta) * observations[j].x) +
        (cos(par_theta) * observations[j].y);
      t_obs_list.push_back(t_obs);

    }

    // Consider landmarks in the range of the sensor of the particle
    vector<LandmarkObs> pred_lm_list;
    for(int j = 0; j < map_landmarks.landmark_list.size(); j++) {

      Map::single_landmark_s lm = map_landmarks.landmark_list[j];

      if(fabs(static_cast<double>(lm.x_f) - par_x) <= sensor_range &&
         fabs(static_cast<double>(lm.y_f) - par_y) <= sensor_range) {

        pred_lm_list.push_back(LandmarkObs { lm.id_i, static_cast<double>(lm.x_f), static_cast<double>(lm.y_f) });
      }
    }

    // Associate transformed observations with the predicted landmarks using NNA
    dataAssociation(pred_lm_list, t_obs_list);

    // Calculate the weight of particle using MGD
    particles[i].weight = 1.0;

    double s_x = std_landmark[0];
    double s_x_sq = s_x * s_x;
    double s_y = std_landmark[1];
    double s_y_sq = s_y * s_y;
    double norm = 1.0 / (2.0* M_PI *s_x * s_y);

    for(int j = 0; j < t_obs_list.size(); j++) {
      
      LandmarkObs t_obs = t_obs_list[j];
      
      for(int k = 0; k < pred_lm_list.size(); k++) {

        LandmarkObs pred_lm = pred_lm_list[k];

        if(t_obs.id == pred_lm.id) {

          double x_sq = pow((t_obs.x - pred_lm.x), 2);
          double y_sq = pow((t_obs.y - pred_lm.y), 2);
          double exponent = -1.0 * (x_sq / (2.0*s_x_sq) + y_sq / (2.0 * s_y_sq));
          double obs_weight = 
            norm * exp(exponent);
          particles[i].weight *= obs_weight;
        }
      }
    }
    weight_norm += particles[i].weight;
  }

  // Normalize the weights of all particles
  for(int i = 0; i < num_particles; i++) {
    if(weight_norm != 0) {
      particles[i].weight /= weight_norm;
    }
    weights[i] = particles[i].weight;
  }

}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	

  vector<Particle> res_par_list;

  uniform_int_distribution<int> par_id_list(0, num_particles - 1);

  int par_id = par_id_list(gen);

  double beta = 0.0;

  double max_w = 2.0 * *max_element(weights.begin(), weights.end());

  for(int i = 0; i < num_particles; i++) {

    uniform_real_distribution<double> random_w(0.0, max_w);
    beta += random_w(gen);

    while(beta > weights[par_id]) {

      beta -= weights[par_id];
      par_id = (par_id + 1) % num_particles;
    }
    
    res_par_list.push_back(particles[par_id]);
  }

  particles = res_par_list;

}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates

    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;

    return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
