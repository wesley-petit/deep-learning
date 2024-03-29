#ifndef RAVEN_BOT_H
#define RAVEN_BOT_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Bot.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:
//-----------------------------------------------------------------------------
#include <vector>
#include <iosfwd>
#include <map>

#include "game/MovingEntity.h"
#include "misc/utils.h"
#include "Raven_TargetingSystem.h"

#include "Spawn_TeamWeapon.h"


class Raven_PathPlanner;
class Raven_Steering;
class Raven_Game;
class Regulator;
class Raven_Weapon;
struct Telegram;
class Raven_Bot;
class Goal_Think;
class Raven_WeaponSystem;
class Raven_SensoryMemory;




class Raven_Bot : public MovingEntity
{
protected:

  enum Status{alive, dead, spawning};

protected:

  //alive, dead or spawning?
  Status                             m_Status;

  //a pointer to the world data
  Raven_Game*                        m_pWorld;

  //this object handles the arbitration and processing of high level goals
  Goal_Think*                        m_pBrain;

  //this is a class that acts as the bots sensory memory. Whenever this
  //bot sees or hears an opponent, a record of the event is updated in the 
  //memory.
  Raven_SensoryMemory*               m_pSensoryMem;

  //the bot uses this object to steer
  Raven_Steering*                    m_pSteering;

  //the bot uses this to plan paths
  Raven_PathPlanner*                 m_pPathPlanner;

  //this is responsible for choosing the bot's current target
  Raven_TargetingSystem*             m_pTargSys;

  //this handles all the weapons. and has methods for aiming, selecting and
  //shooting them
  Raven_WeaponSystem*                m_pWeaponSys;


  // Tableau des spawns possible pour un bot
  std::vector<Spawn_TeamWeapon*>	m_SpawnsTeamWeapon;

  //A regulator object limits the update frequency of a specific AI component
  Regulator*                         m_pWeaponSelectionRegulator;
  Regulator*                         m_pGoalArbitrationRegulator;
  Regulator*                         m_pTargetSelectionRegulator;
  Regulator*                         m_pTriggerTestRegulator;
  Regulator*                         m_pVisionUpdateRegulator;

  //the bot's health. Every time the bot is shot this value is decreased. If
  //it reaches zero then the bot dies (and respawns)
  int                                m_iHealth;
  
  //the bot's maximum health value. It starts its life with health at this value
  int                                m_iMaxHealth;

  //each time this bot kills another this value is incremented
  int                                m_iScore;
  
  //the direction the bot is facing (and therefore the direction of aim). 
  //Note that this may not be the same as the bot's heading, which always
  //points in the direction of the bot's movement
  Vector2D                           m_vFacing;

  //a bot only perceives other bots within this field of view
  double                             m_dFieldOfView;
  
  //to show that a player has been hit it is surrounded by a thick 
  //red circle for a fraction of a second. This variable represents
  //the number of update-steps the circle gets drawn
  int                                m_iNumUpdatesHitPersistant;

  //set to true when the bot is hit, and remains true until 
  //m_iNumUpdatesHitPersistant becomes zero. (used by the render method to
  //draw a thick red circle around a bot to indicate it's been hit)
  bool                               m_bHit;

  //set to true when a human player takes over control of the bot
  bool                               m_bPossessed;

  // Nom de la team du bot pour reconnaitres ses conf�reres, s'il en a une, sinon il est vide
  std::string						 m_sNameTeam;

  // Contient les bots de la team du bot actueel
  std::list<Raven_Bot*>				 m_Teammates;

  // Leader de la team
  Raven_Bot*						 m_pRavenBotLeader;

  //a vertex buffer containing the bot's geometry
  std::vector<Vector2D>              m_vecBotVB;
  //the buffer for the transformed vertices
  std::vector<Vector2D>              m_vecBotVBTrans;

  //apprentissage. 
  //donnee � enregistrer d�crivant une situation de comportement de l'agent 
  std::vector<double> m_vecObservation; //distance-target, visibilite, quantite-arme, type arme, son niveau de vie
  std::vector<double> m_vecTarget; //classes sous forme d'un vecteur de sortie. 

  bool Shooting;



  //bots shouldn't be copied, only created or respawned
  Raven_Bot(const Raven_Bot&);
  Raven_Bot& operator=(const Raven_Bot&);

  //this method is called from the update method. It calculates and applies
  //the steering force for this time-step.
  void          UpdateMovement();

  //initializes the bot's VB with its geometry
  void          SetUpVertexBuffer();


public:
  
  // Le bot n'a de team que si on lui pr�cise dans le constructeur ou dans la fonction SetTeam()
  Raven_Bot(Raven_Game* world, Vector2D pos, std::string nameTeam);
  virtual ~Raven_Bot();

  //the usual suspects
  void         Render();
  void         Update();
  bool         HandleMessage(const Telegram& msg);
  void         Write(std::ostream&  os)const{/*not implemented*/}
  void         Read (std::ifstream& is){/*not implemented*/}

  //this rotates the bot's heading until it is facing directly at the target
  //position. Returns false if not facing at the target.
  bool          RotateFacingTowardPosition(Vector2D target);
 
  //methods for accessing attribute data
  int           Health()const{return m_iHealth;}
  int           MaxHealth()const{return m_iMaxHealth;}
  void          ReduceHealth(unsigned int val);
  void          IncreaseHealth(unsigned int val);
  void          RestoreHealthToMaximum();

  int           Score()const{return m_iScore;}
  void          IncrementScore(){++m_iScore;}

  Vector2D      Facing()const{return m_vFacing;}
  double        FieldOfView()const{return m_dFieldOfView;}

  bool          isPossessed()const{return m_bPossessed;}
  bool          isDead()const{return m_Status == dead;}
  bool          isAlive()const{return m_Status == alive;}
  bool          isSpawning()const{return m_Status == spawning;}
  
  void          SetSpawning(){m_Status = spawning;}
  void          SetDead(){m_Status = dead;}
  void          SetAlive(){m_Status = alive;}

  // D�finie le nom de la team et le leader lorsque le joueur choisie une cible lors d'une partie
  void			SetNameTeam(const std::string& newNameTeam, Raven_Bot* leader = 0) { m_sNameTeam = newNameTeam; m_pRavenBotLeader = leader; }

  // D�finie les teamates du bot
  void			SetTeamates(const std::list<Raven_Bot*> listTeam) { m_Teammates = listTeam; }

  //returns a value indicating the time in seconds it will take the bot
  //to reach the given position at its current speed.
  double        CalculateTimeToReachPosition(Vector2D pos)const; 

  //returns true if the bot is close to the given position
  bool          isAtPosition(Vector2D pos)const;


  //interface for human player
  void          FireWeapon(Vector2D pos);
  void          ChangeWeapon(unsigned int type);
  void          TakePossession();
  void          Exorcise();

  bool			isShooting()const { return Shooting; }
  void			setShootingFalse() { Shooting = FALSE; }

  //spawns the bot at the given position
  void          Spawn(Vector2D pos);
  
  //returns true if this bot is ready to test against all triggers
  bool          isReadyForTriggerUpdate()const;

  //returns true if the bot has line of sight to the given position.
  bool          hasLOSto(Vector2D pos)const;

  //returns true if this bot can move directly to the given position
  //without bumping into any walls
  bool          canWalkTo(Vector2D pos)const;

  //similar to above. Returns true if the bot can move between the two
  //given positions without bumping into any walls
  bool          canWalkBetween(Vector2D from, Vector2D to)const;

  //returns true if there is space enough to step in the indicated direction
  //If true PositionOfStep will be assigned the offset position
  bool          canStepLeft(Vector2D& PositionOfStep)const;
  bool          canStepRight(Vector2D& PositionOfStep)const;
  bool          canStepForward(Vector2D& PositionOfStep)const;
  bool          canStepBackward(Vector2D& PositionOfStep)const;

  
  Raven_Game* const                  GetWorld(){return m_pWorld;} 
  Raven_Steering* const              GetSteering(){return m_pSteering;}
  Raven_PathPlanner* const           GetPathPlanner(){return m_pPathPlanner;}
  Goal_Think* const                  GetBrain(){return m_pBrain;}
  const Raven_TargetingSystem* const GetTargetSys()const{return m_pTargSys;}
  Raven_TargetingSystem* const       GetTargetSys(){return m_pTargSys;}
  Raven_Bot* const                   GetTargetBot()const{return m_pTargSys->GetTarget();}
  Raven_WeaponSystem* const          GetWeaponSys()const{return m_pWeaponSys;}
  Raven_SensoryMemory* const         GetSensoryMem()const{return m_pSensoryMem;}

  std::string						 GetNameTeam()const { return m_sNameTeam; } // Donne le nom de la team du bot
  std::list<Raven_Bot*>				 GetTeammates()const { return m_Teammates; } // Donne la liste des teamates du bot

  bool								 IsInATeam() { return GetNameTeam() != ""; } // Appartient � une team
  double							 GetWeaponHighestAmmo(); // Retourne l'arme avec le plus de munition

  void								 SetTargetBot(Raven_Bot* b)const { m_pTargSys->SetTarget(b); }

  std::vector<double> GetDataShoot() { return m_vecObservation; }
  std::vector<double> GetTargetShoot() { return m_vecTarget; }

};




#endif