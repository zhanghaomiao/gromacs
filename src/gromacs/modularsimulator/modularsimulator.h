/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2019,2020, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \libinternal \file
 * \brief Provides the modular simulator.
 *
 * Defines the ModularSimulator class. Provides checkUseModularSimulator() utility function
 * to determine whether the ModularSimulator should be used.
 *
 * \author Pascal Merz <pascal.merz@me.com>
 * \ingroup module_modularsimulator
 *
 * This header is currently the only part of the modular simulator module which is exposed.
 * Mdrunner creates an object of type ModularSimulator (via SimulatorBuilder), and calls its
 * run() method. Mdrunner also calls checkUseModularSimulator(...), which in turns calls a
 * static method of ModularSimulator. This could easily become a free function if this requires
 * more exposure than otherwise necessary.
 */
#ifndef GROMACS_MODULARSIMULATOR_MODULARSIMULATOR_H
#define GROMACS_MODULARSIMULATOR_MODULARSIMULATOR_H

#include "gromacs/mdrun/isimulator.h"

struct CheckpointHeaderContents;
struct t_fcdata;
struct t_trxframe;

namespace gmx
{
class ModularSimulatorAlgorithmBuilder;
class ReadCheckpointDataHolder;

/*! \libinternal
 * \ingroup module_modularsimulator
 * \brief The modular simulator
 *
 * Based on the input given, this simulator builds independent elements and
 * signallers and stores them in a respective vector. The run function
 * runs the simulation by, in turn, building a task list from the elements
 * for a predefined number of steps, then running the task list, and repeating
 * until the stop criterion is fulfilled.
 */
class ModularSimulator final : public ISimulator
{
public:
    //! Run the simulator
    void run() override;

    //! Check for disabled functionality
    static bool isInputCompatible(bool                             exitOnFailure,
                                  const t_inputrec*                inputrec,
                                  bool                             doRerun,
                                  const gmx_mtop_t&                globalTopology,
                                  const gmx_multisim_t*            ms,
                                  const ReplicaExchangeParameters& replExParams,
                                  const t_fcdata*                  fcd,
                                  bool                             doEssentialDynamics,
                                  bool                             doMembed);

    //! Read everything that can be stored in t_trxframe from a checkpoint file
    static void readCheckpointToTrxFrame(t_trxframe*                     fr,
                                         ReadCheckpointDataHolder*       readCheckpointDataHolder,
                                         const CheckpointHeaderContents& checkpointHeaderContents);

    // Only builder can construct
    friend class SimulatorBuilder;

private:
    //! Constructor
    ModularSimulator(std::unique_ptr<LegacySimulatorData>      legacySimulatorData,
                     std::unique_ptr<ReadCheckpointDataHolder> checkpointDataHolder);

    //! Populate algorithm builder with elements
    void addIntegrationElements(ModularSimulatorAlgorithmBuilder* builder);

    //! Check for disabled functionality (during construction time)
    void checkInputForDisabledFunctionality();

    //! Pointer to legacy simulator data (TODO: Can we avoid using unique_ptr? #3628)
    std::unique_ptr<LegacySimulatorData> legacySimulatorData_;
    //! Input checkpoint data
    std::unique_ptr<ReadCheckpointDataHolder> checkpointDataHolder_;
};

/*!
 * \brief Whether or not to use the ModularSimulator
 *
 * GMX_DISABLE_MODULAR_SIMULATOR environment variable allows to disable modular simulator for
 * all uses.
 *
 * See ModularSimulator::isInputCompatible() for function signature.
 *
 * \ingroup module_modularsimulator
 */
template<typename... Ts>
auto checkUseModularSimulator(Ts&&... args)
        -> decltype(ModularSimulator::isInputCompatible(std::forward<Ts>(args)...))
{
    return ModularSimulator::isInputCompatible(std::forward<Ts>(args)...)
           && getenv("GMX_DISABLE_MODULAR_SIMULATOR") == nullptr;
}

} // namespace gmx

#endif // GROMACS_MODULARSIMULATOR_MODULARSIMULATOR_H
