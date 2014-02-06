/*
 *  Avida2Driver.cc
 *  avida
 *
 *  Created by David on 12/11/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors: David M. Bryson <david@programerror.com>
 *
 */

#include "Avida3Driver.h"

#include "avida/core/Context.h"
#include "avida/core/Universe.h"
#include "avida/systematics/Group.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>

Avida3Driver::Avida3Driver(Avida::Universe* universe, Avida::Feedback& feedback)
  : m_universe(universe), m_feedback(feedback), m_done(false)
{
  Avida::GlobalObjectManager::Register(this);
}

Avida3Driver::~Avida3Driver()
{
  Avida::GlobalObjectManager::Unregister(this);
  delete m_universe;
}


void Avida3Driver::Run()
{
  const int rand_seed = cfg->RANDOM_SEED.Get();
  std::cout << "Random Seed: " << rand_seed;
  if (rand_seed != world->GetRandom().Seed()) std::cout << " -> " << world->GetRandom().Seed();
  std::cout << std::endl;
  
  if (cfg->VERBOSITY.Get() > VERBOSE_NORMAL) std::cout << "Data Directory: " << opath << std::endl;
  
  std::cout << std::endl;

  
  cPopulation& population = m_world->GetPopulation();
  cStats& stats = m_world->GetStats();
  
  void (cPopulation::*ActiveProcessStep)(cAvidaContext& ctx, double step_size, int cell_id) = &cPopulation::ProcessStep;
  if (m_world->GetConfig().SPECULATIVE.Get() &&
      m_world->GetConfig().THREAD_SLICING_METHOD.Get() != 1 && !m_world->GetConfig().IMPLICIT_REPRO_END.Get()) {
    ActiveProcessStep = &cPopulation::ProcessStepSpeculative;
  }
  
  cAvidaContext& ctx = m_world->GetDefaultContext();
  Avida::Context new_ctx(this, &m_world->GetRandom());
  
  while (!m_done) {
    m_world->GetEvents(ctx);
    if(m_done == true) break;
    
    // Increment the Update.
    stats.IncCurrentUpdate();
    
    population.ProcessPreUpdate();

    // Handle all data collection for previous update.
    if (stats.GetUpdate() > 0) {
      // Tell the stats object to do update calculations and printing.
      stats.ProcessUpdate();
    }
    
    // Process the update.
    // query the world to calculate the exact size of this update:
    const int UD_size = GetConfig().AVE_TIME_SLICE.Get() * GetPopulation().GetNumOrganisms()
    const double step_size = 1.0 / (double) UD_size;
    
    for (int i = 0; i < UD_size; i++) {
      if(population.GetNumOrganisms() == 0) {
        break;
      }
      (population.*ActiveProcessStep)(ctx, step_size, population.ScheduleOrganism());
    }
    
    // end of update stats...
    population.ProcessPostUpdate(ctx);
    
		m_world->ProcessPostUpdate(ctx);
        
    // No viewer; print out status for this update....
    if (m_world->GetVerbosity() > VERBOSE_SILENT) {
      cout.setf(ios::left);
      cout.setf(ios::showpoint);
      cout << "UD: " << setw(6) << stats.GetUpdate() << "  ";
      cout << "Gen: " << setw(9) << setprecision(7) << stats.SumGeneration().Average() << "  ";
      cout << "Fit: " << setw(9) << setprecision(7) << stats.GetAveFitness() << "  ";
      cout << "Orgs: " << setw(6) << population.GetNumOrganisms() << "  ";
      if (m_world->GetVerbosity() == VERBOSE_ON || m_world->GetVerbosity() == VERBOSE_DETAILS) {
        cout << "Merit: " << setw(9) << setprecision(7) << stats.GetAveMerit() << "  ";
        cout << "Thrd: " << setw(6) << stats.GetNumThreads() << "  ";
      }
      if (m_world->GetVerbosity() >= VERBOSE_DEBUG) {
        cout << "Spec: " << setw(6) << setprecision(4) << stats.GetAveSpeculative() << "  ";
        cout << "SWst: " << setw(6) << setprecision(4) << (((double)stats.GetSpeculativeWaste() / (double)UD_size) * 100.0) << "%  ";
      }

      cout << endl;
    }
    
    
    m_new_world->PerformUpdate(new_ctx, stats.GetUpdate());
    
    // Exit conditons...
    if((population.GetNumOrganisms()==0) && m_world->AllowsEarlyExit()) {
			m_done = true;
		}
  }
}

void Avida3Driver::Abort(Avida::AbortCondition condition)
{
  exit(condition);
}


