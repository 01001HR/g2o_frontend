#include "matcher.h"


using namespace g2o;

namespace match_this
{

float MatcherResult::matchingScore() const
{
    return _matchingScore;
}


MatcherResult::~MatcherResult() {}


Matcher::~Matcher()
{
    clear();
    clearMatchResults();
}


void Matcher::clear() {}


void Matcher::clearMatchResults()
{
    for(ResultContainer::iterator it = _matchResults.begin(); it != _matchResults.end(); ++it)
    {
        delete *it;
    }
    _matchResults.clear();
}


void Matcher::match(HyperGraph::Vertex* ref, HyperGraph::Vertex* curr) {}
}
