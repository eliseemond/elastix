/*=========================================================================
 *
 *  Copyright UMC Utrecht and contributors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "xoutsimple.h"

namespace xoutlibrary
{

/**
 * **************** AddOutput (std::ostream) ********************
 */

int
xoutsimple::AddOutput(const char * name, std::ostream * output)
{
  return this->AddTargetCell(name, output);

} // end AddOutput


/**
 * **************** AddOutput (xoutsimple) **********************
 */

int
xoutsimple::AddOutput(const char * name, Superclass * output)
{
  return this->AddTargetCell(name, output);

} // end AddOutput


/**
 * ***************** RemoveOutput *******************************
 */

int
xoutsimple::RemoveOutput(const char * name)
{
  return this->RemoveTargetCell(name);

} // end RemoveOutput


/**
 * **************** SetOutputs (std::ostreams) ******************
 */

void
xoutsimple::SetOutputs(const CStreamMapType & outputmap)
{
  this->SetTargetCells(outputmap);

} // end SetOutputs


/**
 * **************** SetOutputs (xoutobjects) ********************
 */

void
xoutsimple::SetOutputs(const XStreamMapType & outputmap)
{
  this->SetTargetCells(outputmap);

} // end SetOutputs()


/**
 * **************** GetOutputs (map of xoutobjects) *************
 */

auto
xoutsimple::GetXOutputs(void) -> const XStreamMapType &
{
  return this->m_XTargetCells;

} // end GetXOutputs()

/**
 * **************** GetOutputs (map of c-streams) ***************
 */

auto
xoutsimple::GetCOutputs(void) -> const CStreamMapType &
{
  return this->m_CTargetCells;

} // end GetCOutputs()

} // end namespace xoutlibrary
