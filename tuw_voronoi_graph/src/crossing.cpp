/*
 * Copyright (c) 2017, <copyright holder> <email>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <voronoi_segmentation/crossing.h>
#include <limits>


Crossing::Crossing(std::shared_ptr< std::vector< std::pair< int, int > > > _segment_points) : segments_start_(), segments_end_()
{
  int min_x = __INT_MAX__;
  int max_x = 0;
  int min_y = __INT_MAX__;
  int max_y = 0;
  
  for(auto it = _segment_points->begin(); it != _segment_points->end(); ++it)
  {
	if(it->first > max_x)
	  max_x = it->first;
	
	if(it->first < min_x)
	  min_x = it->first;
	
	if(it->second > max_y)
	  max_y = it->second;
	
	if(it->second < min_y)
	  min_y = it->second;
  }
  
  surroundingPoints_ = _segment_points;
  center_.first = ((float)(max_x + min_x))/2.0;
  center_.second = ((float)(max_y + min_y))/2.0;
  
}

bool Crossing::TryAddSegment(std::shared_ptr< Segment > _seg)
{	
  for(auto it = surroundingPoints_->begin(); it != surroundingPoints_->end(); ++it)
  {
	if(it->first == _seg->getStart().first && it->second == _seg->getStart().second)
	{
	  for(auto it_segs = segments_start_.begin(); it_segs != segments_start_.end(); ++it_segs)
	  {
		_seg->AddPredecessor((*it_segs));
		(*it_segs)->AddPredecessor(_seg);
	  }
	  for(auto it_segs = segments_end_.begin(); it_segs != segments_end_.end(); ++it_segs)
	  {
		_seg->AddPredecessor((*it_segs));
		(*it_segs)->AddSuccessor(_seg);
	  }
	  
	  //HACK APPEND CROSSING CENTER
	  auto p = _seg->GetPath();
	  p->insert(p->begin(), center_);
	  _seg->SetPath(p);
	  segments_start_.push_back(_seg);
	  return true;
	}
	
	if(it->first == _seg->getEnd().first && it->second == _seg->getEnd().second)
	{
	  for(auto it_segs = segments_start_.begin(); it_segs != segments_start_.end(); ++it_segs)
	  {
		_seg->AddSuccessor((*it_segs));
		(*it_segs)->AddPredecessor(_seg);
		
	  }
	  for(auto it_segs = segments_end_.begin(); it_segs != segments_end_.end(); ++it_segs)
	  {
		_seg->AddSuccessor((*it_segs));
		(*it_segs)->AddSuccessor(_seg);
	  }
	  
	  //HACK APPEND CROSSING CENTER
	  auto p = _seg->GetPath();
	  p->push_back(center_);
	  _seg->SetPath(p);
	  segments_end_.push_back(_seg);
	  return true;
	}
  }
  
  return false;
}

std::pair< float, float > Crossing::getCenter()
{
  return center_;
}
