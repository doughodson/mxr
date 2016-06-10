
#include "openeaagles/ubf/soar/SoarArbiter.h"
#include "openeaagles/ubf/soar/SoarState.h"

#include "openeaagles/base/List.h"
#include "openeaagles/base/String.h"

#include "openeaagles/ubf/behaviors/PlaneAction.h"

#include "openeaagles/ubf/behaviors/Factory.h"
#include "openeaagles/ubf/lua/factory.h"

#include "openeaagles/base/PairStream.h"
#include "openeaagles/base/Number.h"
#include "openeaagles/base/Integer.h"
#include "openeaagles/base/Float.h"

#include "openeaagles/base/ubf/Action.h"
#include "openeaagles/base/ubf/State.h"

#include "soar/sml_Client.h"

namespace oe {
namespace soar {

IMPLEMENT_SUBCLASS(SoarArbiter, "SoarArbiter")
EMPTY_COPYDATA(SoarArbiter)
EMPTY_SERIALIZER(SoarArbiter)

BEGIN_SLOTTABLE(SoarArbiter)
   "soarFileName"
END_SLOTTABLE(SoarArbiter)

BEGIN_SLOT_MAP(SoarArbiter)
   ON_SLOT(1, setSlotSoarFileName, base::String)
END_SLOT_MAP()

SoarArbiter::SoarArbiter()
{
    STANDARD_CONSTRUCTOR()
    fileName = new base::String();
}

void SoarArbiter::deleteData()
{
    if (fileName != nullptr)   { fileName->unref(); fileName = nullptr; }
    kernel->Shutdown();
    delete kernel;
}

bool SoarArbiter::setSlotSoarFileName(base::String* const x)
{
    if (fileName != nullptr)  { fileName->unref();  fileName = nullptr; }
    if (x != nullptr)         { fileName = x->clone(); }
    initSoar();
    return true;
}

base::Object* SoarArbiter::getSlotByIndex(const int si)
{
   return BaseClass::getSlotByIndex(si);
}

//generates an action by calling a SOAR script.
base::ubf::Action* SoarArbiter::genAction( const base::ubf::State* const state, const double dt )
{
    static double l_timer = 1.0;
    l_timer += dt;
    // create list for action set
    base::List* actionSet = new base::List();

    // fill out list of recommended actions by behaviors
    if (l_timer >= 1.0) {
        Commit( (oe::behaviors::PlaneState*) state );     // ddh problem
        Run();
        Retrieve();
        l_timer = 0.0;
    }
    // return action to perform generated by our base class
    return BaseClass::genAction(state, dt);
}

// generates a complex action from a set of actions. By default, it selects the action with the highest vote
base::ubf::Action* SoarArbiter::genComplexAction(base::List* const actionSet)
{
    oe::behaviors::PlaneAction* complexAction = new oe::behaviors::PlaneAction;

    unsigned int maxPitchVote = 0;
    unsigned int maxRollVote = 0;
    unsigned int maxHeadingVote = 0;
    unsigned int maxFireVote = 0;
    unsigned int maxThrottleVote = 0;
    unsigned int maxPitchTrimVote = 0;

    // process entire action set
    const base::List::Item* item = actionSet->getFirstItem();
    while (item != nullptr) {
        const oe::behaviors::PlaneAction* action = dynamic_cast<const oe::behaviors::PlaneAction*>(item->getValue());
        if (action != nullptr) {
            if (action->isHeadingChanged() && action->getVote() > maxHeadingVote) {
                complexAction->setHeading(action->getHeading());
                maxHeadingVote = action->getVote();
            }
            if (action->isPitchChanged() && action->getVote() > maxPitchVote) {
                complexAction->setPitch(action->getPitch());
                maxPitchVote = action->getVote();
            }
            if (action->isRollChanged() && action->getVote() > maxRollVote) {
                complexAction->setRoll(action->getRoll());
                maxRollVote = action->getVote();
            }
            if (action->isFireMissileChanged() && action->getVote() > maxFireVote) {
                complexAction->setFireMissile( action->getFireMissile() );
                maxFireVote = action->getVote();
            }
            if (action->isThrottleChanged() && action->getVote() > maxThrottleVote) {
                complexAction->setThrottle( action->getThrottle() );
                maxThrottleVote = action->getVote();
            }
            if (action->isPitchTrimChanged() && action->getVote() > maxPitchTrimVote) {
                complexAction->setPitchTrim( action->getPitchTrim() );
                maxPitchTrimVote = action->getVote();
            }
        } else {
            std::cout << "Action NOT a PlaneAction\n";
        }
        // next action
        item = item->getNext();
    }
    trimChangeValidation(complexAction);
    return complexAction;
}

void SoarArbiter::trimChangeValidation(base::ubf::Action* const complexAction)
{
    oe::behaviors::PlaneAction* action = static_cast<oe::behaviors::PlaneAction*>(complexAction);

    if (action->isPitchChanged() && action->isPitchTrimChanged()) {
        //ignore pitch trim ONLY if the change goes against the action we want to
        //take with the control stick.
        if (action->getPitch() > 0) {
            if (action->getPitchTrim() < 0) {
                action->setPitchTrim(0);
            }
        } else {
            if (action->getPitchTrim() > 0) {
                action->setPitchTrim(0);
            }
        }
    }
    return;
}

void SoarArbiter::initSoar()
{
    kernel = sml::Kernel::CreateKernelInNewThread();
    if (kernel->HadError()) {
        std::cout << kernel->GetLastErrorDescription() << std::endl;
    }

    agent = kernel->CreateAgent(getFactoryName());
    if (kernel->HadError()) {
        std::cout << kernel->GetLastErrorDescription() << std::endl;
    }

    agent->LoadProductions(getSoarFileName()->getCopyString());
    if (agent->HadError()) {
        std::cout << agent->GetLastErrorDescription() << std::endl;
    }

    soarState = new SoarState(agent);
    inputLink = agent->GetInputLink();
    id        = agent->CreateIdWME(inputLink, SoarState::getFactoryName());

    soarState->setRoll              (agent->CreateFloatWME(id, "roll-is",        0));
    soarState->setPitch             (agent->CreateFloatWME(id, "pitch-is",       0));
    soarState->setHeading           (agent->CreateFloatWME(id, "heading-is",     0));
    soarState->setRollRate          (agent->CreateFloatWME(id, "roll-rate-is",   0));
    soarState->setPitchRate         (agent->CreateFloatWME(id, "pitch-rate-is",  0));
    soarState->setYawRate           (agent->CreateFloatWME(id, "yaw-rate-is",    0));
    soarState->setAltitude          (agent->CreateFloatWME(id, "altitude-is",    0));
    soarState->setThrottle          (agent->CreateFloatWME(id, "throttle-is",    0));
    soarState->setSpeed             (agent->CreateFloatWME(id, "speed-is",       0));
    soarState->setPitchTrim         (agent->CreateFloatWME(id, "pitch-trim-is",  0));

    soarState->setNumTracks         (agent->CreateIntWME(id, "num-tracks-is",        0));
    soarState->setTargetTrack       (agent->CreateIntWME(id, "target-track-is",      0));
    soarState->setNumEngines        (agent->CreateIntWME(id, "num-engines-is",       0));
    
    soarState->setAlive             (agent->CreateIntWME(id, "is-alive",             0));
    soarState->setTracking          (agent->CreateIntWME(id, "is-tracking",          0));
    soarState->setMissileFired      (agent->CreateIntWME(id, "is-missile-fired",     0));
    soarState->setIncomingMissile   (agent->CreateIntWME(id, "is-incoming-missile",  0));
    
    soarState->setPitchToTracked    (agent->CreateFloatWME(id, "pitch-to-tracked-is", 0));
    soarState->setHeadingToTracked  (agent->CreateFloatWME(id, "heading-to-tracked-is", 0));
    soarState->setDistanceToTracked (agent->CreateFloatWME(id, "distance-to-tracked-is", 0));
}

// commit the states of the aircraft.
void SoarArbiter::Commit(const base::ubf::State* const state)
{
    const oe::behaviors::PlaneState* l_state = dynamic_cast<const oe::behaviors::PlaneState*>(state);
    agent->Update(soarState->getRoll(),         l_state->getRoll());
    agent->Update(soarState->getPitch(),        l_state->getPitch());
    agent->Update(soarState->getHeading(),      l_state->getHeading());
    agent->Update(soarState->getRollRate(),     l_state->getRollRate());
    agent->Update(soarState->getPitchRate(),    l_state->getPitchRate());
    agent->Update(soarState->getYawRate(),      l_state->getYawRate());
    agent->Update(soarState->getAltitude(),     l_state->getAltitude());
    agent->Update(soarState->getThrottle(),     l_state->getThrottle());
    agent->Update(soarState->getSpeed(),        l_state->getSpeed());
    agent->Update(soarState->getPitchTrim(),    l_state->getPitchTrim());

    agent->Update(soarState->getNumTracks(),    l_state->getNumTracks());
    agent->Update(soarState->getTargetTrack(),  l_state->getTargetTrack());
    agent->Update(soarState->getNumEngines(),   l_state->getNumEngines());
    
    agent->Update(soarState->getAlive(),            (l_state->isAlive()            == true ) ? ( 1 ) : ( 0 ) );
    agent->Update(soarState->getTracking(),         (l_state->isTracking()         == true ) ? ( 1 ) : ( 0 ) );
    agent->Update(soarState->getMissileFired(),     (l_state->isMissileFired()     == true ) ? ( 1 ) : ( 0 ) );
    agent->Update(soarState->getIncomingMissile(),  (l_state->isIncomingMissile()  == true ) ? ( 1 ) : ( 0 ) );
    
    if (l_state->isTracking() && l_state->getTargetTrack() < l_state->getNumTracks()) {
        agent->Update(soarState->getPitchToTracked(), l_state->getPitchToTracked(l_state->getTargetTrack()));
        agent->Update(soarState->getHeadingToTracked(), l_state->getHeadingToTracked(l_state->getTargetTrack()));
        agent->Update(soarState->getDistanceToTracked(), l_state->getDistanceToTracked(l_state->getTargetTrack()));
    } else {
        agent->Update(soarState->getPitchToTracked(), 0);
        agent->Update(soarState->getHeadingToTracked(), 0);
        agent->Update(soarState->getDistanceToTracked(), 0);
    }
    // Send the changes to working memory to Soar
    // With 8.6.2 this call is optional as changes are sent automatically.
    agent->Commit();
}

// run Soar
void SoarArbiter::Run()
{
    agent->RunSelfTilOutput();
}

// retrieve information from Soar environment
void SoarArbiter::Retrieve()
{
    sml::Identifier* l_OutputLink = agent->GetOutputLink();
    int numberCommands = agent->GetNumberCommands();
    for (int i = 0 ; i < numberCommands; i++) {
        sml::Identifier* pCommand = agent->GetCommand(i);
        const std::string l_cmdName = pCommand->GetCommandName();
        //const char * c_name_s   = pCommand->GetValueAsString();
        
        if (l_cmdName == "behaviors") {
            std::string l_behaviorName = pCommand->GetParameterValue("name");
            if (l_behaviorName != "init") {
                base::Object* l_bhv = nullptr;
                if (l_bhv == nullptr) {
                    l_bhv = behaviors::factory(l_behaviorName);
                }
                if (l_bhv == nullptr) {
                    l_bhv = lua::factory(l_behaviorName);
                }

                int j = 1;
                int bla = pCommand->GetNumberChildren();
                sml::WMElement* l_WME = pCommand->GetChild(j);
                while(l_WME != nullptr) {
                    base::Object* l_param = nullptr;
                    std::string l_valueType = l_WME->GetValueType();

                    if (l_valueType == "string") {
                        l_param = new base::String(l_WME->ConvertToStringElement()->GetValue());
                    } else if (l_valueType == "int") {
                        l_param = new base::Integer(l_WME->ConvertToIntElement()->GetValue());
                    } else if (l_valueType == "double") {
                        l_param = new base::Number( l_WME -> ConvertToFloatElement() -> GetValue() );
                    } else if (l_valueType == "id") {
                        // this is somewhat LuaBehavior specific
                        int k = 0;
                        sml::WMElement* l_WMEoptPairStream = l_WME->ConvertToIdentifier()->GetChild(k);
                        base::Pair* l_pair = nullptr;
                        l_param = new base::PairStream();

                        while(l_WMEoptPairStream != nullptr) {
                            base::Object* l_pairobj = nullptr;

                            l_valueType = l_WMEoptPairStream->GetValueType();

                            if (l_valueType == "string") {
                                l_pairobj = new base::String(l_WMEoptPairStream->ConvertToStringElement()->GetValue());
                            } else if(l_valueType == "int") {
                                l_pairobj = new base::Integer(l_WMEoptPairStream->ConvertToIntElement()->GetValue());
                            } else if (l_valueType == "double") {
                                l_pairobj = new base::Number(l_WMEoptPairStream->ConvertToFloatElement()->GetValue());
                            }

                            l_pair = new base::Pair(l_WMEoptPairStream->GetAttribute(), l_pairobj);
                            static_cast<base::PairStream*>(l_param)->addTail(l_pair);

                            l_WMEoptPairStream = l_WME->ConvertToIdentifier()->GetChild(++k);
                        }
                    }
                    l_bhv->setSlotByName(l_WME->GetAttribute(), l_param);
                    l_WME = pCommand->GetChild(++j);
                }

                if (l_bhv != nullptr) {
                    addBehavior(static_cast<base::ubf::Behavior*>(l_bhv));
                }
            }
        }
    }
}

}
}
