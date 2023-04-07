void main()
{
	//INIT ECONOMY--------------------------------------
	Hive ce = CreateHive();
	if ( ce )
		ce.InitOffline();

	//DATE RESET AFTER ECONOMY INIT-------------------------
	int year, month, day, hour, minute;
	int reset_month = 9, reset_day = 20;
	GetGame().GetWorld().GetDate(year, month, day, hour, minute);

	if ((month == reset_month) && (day < reset_day))
	{
		GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
	}
	else
	{
		if ((month == reset_month + 1) && (day > reset_day))
		{
			GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
		}
		else
		{
			if ((month < reset_month) || (month > reset_month + 1))
			{
				GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
			}
		}
	}

  
}

class CustomMission: MissionServer
{
	void customMapEvents()
	{

	}

	void SetRandomHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( 0.45, 0.65 );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}

	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt;
		playerEnt = GetGame().CreatePlayer( identity, characterName, pos, 0, "NONE" );
		Class.CastTo( m_player, playerEnt );

		GetGame().SelectPlayer( identity, m_player );

		return m_player;
	}

	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		string pName = player.GetIdentity().GetName();

		
		EntityAI itemClothing;
		EntityAI itemEnt;
		ItemBase itemBs;
		float rand;

		itemClothing = player.FindAttachmentBySlotName( "Body" );
		if ( itemClothing )
		{
			SetRandomHealth( itemClothing );
			
			itemEnt = itemClothing.GetInventory().CreateInInventory( "BandageDressing" );
			if ( Class.CastTo( itemBs, itemEnt ) )
				itemBs.SetQuantity( 2 );

			string chemlightArray[] = { "Chemlight_White", "Chemlight_Yellow", "Chemlight_Green", "Chemlight_Red" };
			int rndIndex = Math.RandomInt( 0, 4 );
			itemEnt = itemClothing.GetInventory().CreateInInventory( chemlightArray[rndIndex] );
			SetRandomHealth( itemEnt );

			rand = Math.RandomFloatInclusive( 0.0, 1.0 );
			if ( rand < 0.35 )
				itemEnt = player.GetInventory().CreateInInventory( "Apple" );
			else if ( rand > 0.65 )
				itemEnt = player.GetInventory().CreateInInventory( "Pear" );
			else
				itemEnt = player.GetInventory().CreateInInventory( "Plum" );

			SetRandomHealth( itemEnt );
		}
		
		itemClothing = player.FindAttachmentBySlotName( "Legs" );
		if ( itemClothing )
			SetRandomHealth( itemClothing );
		
		itemClothing = player.FindAttachmentBySlotName( "Feet" );
/*
	arma al spawnear
		vector pos = player.GetPosition();
		EntityAI item = player.SpawnEntityOnGroundPos("SVD", pos);
		
		item.GetInventory().CreateAttachment("AK_Suppressor");
		
		item = player.SpawnEntityOnGroundPos("KazuarOptic", pos);
		item.GetInventory().CreateAttachment("Battery9V");				
		
		player.SpawnEntityOnGroundPos("Mag_SVD_10Rnd", pos);
		player.SpawnEntityOnGroundPos("Mag_SVD_10Rnd", pos);
		player.SpawnEntityOnGroundPos("Mag_SVD_10Rnd", pos);
		player.SpawnEntityOnGroundPos("Mag_SVD_10Rnd", pos);
*/				
	}


	ref map<string, int> sysConfig = new map<string, int>();
	ref map<string, ref map<string, string>>  playersConfig = new map<string, ref map<string, string>>();
	ref map<string, string> adminConfig = new map<string, string>();
	//players that have been active during the current session
	ref map<string, ref map<string, string>>  activePlayersConfig = new map<string, ref map<string, string>>();
	ref map<string, string> steamIdByName = new map<string,string>();
	ref map<string, int> killFeedOnRequest = new map<string, int>();
	ref map<string, int> killFeedOffRequest = new map<string, int>();
	//enforced script TStringSet is not efficient
	ref TStringMap mutedPlayersMap = new TStringMap;
	ref TStringMap adminPlayersIds = new TStringMap;
	ref TStringMap modPlayersIds = new TStringMap;

  override void OnInit()
	{
		super.OnInit();

		customMapEvents();

		sysConfigSingleton();

		loadAdminConfig();

		loadPlayers();

		scheduleTasks();

	}

	override void OnMissionFinish()	
	{
		saveServerStatus();
		super.OnMissionFinish();
	}

	void sysConfigSingleton()
	{
		sysConfig.Set("killFeedNotifyMinutes", 1);
		sysConfig.Set("autoSaveMinutes", 4);
		sysConfig.Set("kickPlayersOnDuplicateName", 1);
		sysConfig.Set("serverRestartMinutes", 384);
		sysConfig.Set("debug_enabled", 1);

		map<string, string> tempSysConfig = new map<string, string>();

		JsonFileLoader<ref map<string, string>>.JsonLoadFile( "$profile:sysConfig.json", tempSysConfig );

		if(tempSysConfig.Contains("killFeedNotifyMinutes"))
			sysConfig.Set("killFeedNotifyMinutes", tempSysConfig.Get("killFeedNotifyMinutes").ToInt());

		if(tempSysConfig.Contains("autoSaveMinutes"))
			sysConfig.Set("autoSaveMinutes", tempSysConfig.Get("autoSaveMinutes").ToInt());


		if(tempSysConfig.Contains("kickPlayersOnDuplicateName"))
			sysConfig.Set("kickPlayersOnDuplicateName", tempSysConfig.Get("kickPlayersOnDuplicateName").ToInt());

		if(tempSysConfig.Contains("kickPlayersOnDuplicateName"))
			sysConfig.Set("kickPlayersOnDuplicateName", tempSysConfig.Get("kickPlayersOnDuplicateName").ToInt());

		if(tempSysConfig.Contains("debug_enabled"))
			sysConfig.Set("debug_enabled", tempSysConfig.Get("debug_enabled").ToInt());
		else{
			
			tempSysConfig = new map<string, string>();
			tempSysConfig.Set("killFeedNotifyMinutes", "" + sysConfig.Get("killFeedNotifyMinutes"));
			tempSysConfig.Set("autoSaveMinutes", "" + sysConfig.Get("autoSaveMinutes"));
			tempSysConfig.Set("kickPlayersOnDuplicateName", "" + sysConfig.Get("kickPlayersOnDuplicateName"));
			tempSysConfig.Set("serverRestartMinutes", "" + sysConfig.Get("serverRestartMinutes"));
			tempSysConfig.Set("debug_enabled", "" + sysConfig.Get("debug_enabled"));
			
			JsonFileLoader<map<string, string>>.JsonSaveFile( "$profile:sysConfig.json", TStringMap.Cast(tempSysConfig) );
		}

		tempSysConfig = NULL;
	}


	void loadAdminConfig()
	{
		adminConfig.Insert("mutedPlayers", "");
		adminConfig.Insert("modPlayers", "");
		adminConfig.Insert("adminPlayers", "");

		map<string, string> tempAdminConfig = new map<string, string>();
		JsonFileLoader<ref map<string, string>>.JsonLoadFile( "$profile:adminConfig.json", tempAdminConfig );

		TStringArray aCAplyrs;
		int aCidMp = 0;
		bool foundACfg = false;

		if(tempAdminConfig.Contains("mutedPlayers"))
		{
			adminConfig.Set("mutedPlayers", tempAdminConfig.Get("mutedPlayers"));
			aCAplyrs = new TStringArray;
			tempAdminConfig.Get("mutedPlayers").Split(" ", aCAplyrs);

			for( aCidMp = 0; aCidMp < aCAplyrs.Count(); aCidMp++ )
			{
				mutedPlayersMap.Insert(aCAplyrs.Get(aCidMp), "perm");
			}
			aCAplyrs = NULL;
			foundACfg = true;
		}

		if(tempAdminConfig.Contains("adminPlayers"))
		{
			adminConfig.Set("adminPlayers", tempAdminConfig.Get("adminPlayers"));
			aCAplyrs = new TStringArray;1
			tempAdminConfig.Get("adminPlayers").Split(" ", aCAplyrs);

			for( aCidMp = 0; aCidMp < aCAplyrs.Count(); aCidMp++ )
			{
				adminPlayersIds.Insert(aCAplyrs.Get(aCidMp), "");
			}
			aCAplyrs = NULL;
			foundACfg = true;
		}


		if(tempAdminConfig.Contains("modPlayers"))
		{
			adminConfig.Set("modPlayers", tempAdminConfig.Get("modPlayers"));
			aCAplyrs = new TStringArray;
			tempAdminConfig.Get("modPlayers").Split(" ", aCAplyrs);

			for( aCidMp = 0; aCidMp < aCAplyrs.Count(); aCidMp++)
			{
				modPlayersIds.Insert(aCAplyrs.Get(aCidMp), "");
			}
			aCAplyrs = NULL;
			foundACfg = true;
		}
		if( foundACfg == false )
			JsonFileLoader<ref map<string, string>>.JsonSaveFile( "$profile:adminConfig.json", adminConfig );

		aCidMp = 0;

	}

	void loadPlayers()
	{
		JsonFileLoader<ref map<string, ref map<string, string>>>.JsonLoadFile( "$profile:playersConfig.json", playersConfig );

		TStringArray idPArray = playersConfig.GetKeyArray();

		for(int k = 0; k < idPArray.Count(); k ++)
		{

			steamIdByName.Insert(playersConfig.Get(idPArray.Get(k)).Get("name"), idPArray.Get(k));

		}

	}

	void scheduleTasks()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(updateKillFeedConfig, ( sysConfig.Get("killFeedNotifyMinutes")) * 60 * 1000, true);
		//GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(checkCorpses, sysConfig.Get("corpsesCheckMinutes") * 60 * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(saveServerStatus, sysConfig.Get("autoSaveMinutes") * 60 * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(saveServerStatus, sysConfig.Get("serverRestartMinutes") * 60 * 1000, false);

	}

	void savePlayersConfig()
	{
	
		TStringArray idAPArray = activePlayersConfig.GetKeyArray();

		for(int k2 = 0; k2 < idAPArray.Count(); k2 ++)
		{
			playersConfig.Set(idAPArray.Get(k2), activePlayersConfig.Get(idAPArray.Get(k2)));
		}

		JsonFileLoader<ref map<string, ref map<string, string>>>.JsonSaveFile( "$profile:playersConfig.json", playersConfig );
	}

	void saveMutedPlayers()
	{

		string mutedPlayersFormattedOut = "";

		TStringArray mPlIdsvArray = mutedPlayersMap.GetKeyArray();

		for(int mPlIdsv = 0; mPlIdsv < mPlIdsvArray.Count(); mPlIdsv ++)
		{
			if ( mutedPlayersMap.Get(mPlIdsvArray.Get(mPlIdsv)) == "perm"){

				if( mPlIdsv > 0)
					mutedPlayersFormattedOut = mutedPlayersFormattedOut + " ";
				mutedPlayersFormattedOut = mutedPlayersFormattedOut + mPlIdsvArray.Get(mPlIdsv);
			}
		}
		adminConfig.Set("mutedPlayers", mutedPlayersFormattedOut);

		JsonFileLoader<ref map<string, string>>.JsonSaveFile( "$profile:adminConfig.json", adminConfig );

	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		string plyId = identity.GetId();
		string plyCurName = getNameWithoutNumber(identity.GetName());

		if(playersConfig.Contains(plyId))
		{
			if(playersConfig.Get(plyId).Get("name") != plyCurName)
			{
				if (plyCurName == "Survivor")
				{
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameRemoved, 20000, false, player);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameRemoved, 30000, false, player);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameRemoved, 40000, false, player);
				}
				else if(steamIdByName.Contains(plyCurName))
				{

					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 30000, false, identity);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 40000, false, identity);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 50000, false, identity);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 60000, false, identity);
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 70000, false, identity);
					if(sysConfig.Get("kickPlayersOnDuplicateName") == 1)
						GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(kickNameDuplicated, 80000, false, player);
					
				}
				else
				{
					string oldName = playersConfig.Get(plyId).Get("name");
					playersConfig.Get(plyId).Set("name", plyCurName);
					steamIdByName.Remove( oldName );
					steamIdByName.Insert( plyCurName, plyId );
					activePlayersConfig.Set(plyId, playersConfig.Get(plyId));
				}
			}
			else
			{
				activePlayersConfig.Set(plyId, playersConfig.Get(plyId));
			}
			playersConfig.Get(plyId).Set("lastConnection", getDate());	

		}
		else if(plyCurName != "Survivor")
		{	
			if(steamIdByName.Contains(plyCurName))
			{
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 30000, false, identity);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 40000, false, identity);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 50000, false, identity);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 60000, false, identity);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(alertNameDuplicated, 70000, false, identity);
				if(sysConfig.Get("kickPlayersOnDuplicateName") == 1)
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(kickNameDuplicated, 80000, false, player);
			}
			else
			{
			
				ref map<string, string> pConfig = new map<string, string>();

				pConfig.Insert("name", plyCurName);
				pConfig.Insert("killFeedOn", "true");
				pConfig.Insert("localChat", "false");
				pConfig.Insert("lastConnection", getDate());
				pConfig.Insert("kills", "0");
				pConfig.Insert("deaths", "0");

				activePlayersConfig.Insert( plyId, pConfig );
				steamIdByName.Insert( plyCurName, plyId );
			}
		}

	}



	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);
		switch(eventTypeId)
		{
			// Handle user command
			case ChatMessageEventTypeID:
				
				//ClientNewEventParams eventParams;
				ChatMessageEventParams chatParams;
				
				//Class.CastTo(eventParams, params);
				Class.CastTo(chatParams, params);
				
				string msg = string.ToString(chatParams.param3, false, false, false);

				string senderName = string.ToString(chatParams.param2, false, false, false);

				if ( msg.Get(0) == "!" ) return;


				if(!senderName || senderName == "")
					return;		

				if( senderName == "Survivor" )return;

				PlayerBase player = getPlayer(senderName);

				TStringArray strgs = new TStringArray;
				msg.Split(" ", strgs);
				if (msg.Get(0) == "?")
				{
					if(isPlayerMod(player.GetIdentity().GetId()) || isPlayerAdmin(player.GetIdentity().GetId()))
					{
						string pTarget = "";

						switch (strgs.Get(0))
						{

							case "?tp":

								switch(strgs.Get(1))
								{

									case "coord":

										if( strgs.Count() > 4 )
										{
											pTarget = strgs.Get(4);
											pTarget = msg.Substring(msg.IndexOf(pTarget), msg.Length());

											if( isPlayerMod(player.GetIdentity().GetId()) )
											{
												if( steamIdByName.Contains(pTarget) )
												{
													sendPlayerMessage(player, pTarget + " has been teleported.");
													tpToCoord(pTarget, strgs.Get(2), strgs.Get(3));
												}
											}
										}
										return;

									default:
										return;
								}
								return;

								case "?mute":
									if( strgs.Count() > 2 )
									{
										pTarget = strgs.Get(2);
										pTarget = msg.Substring(msg.IndexOf(pTarget), msg.Length());

										if ( steamIdByName.Contains(pTarget) )
										{
											PlayerBase mutedPlayer = getPlayer(pTarget);
											string mutedPlyrId = steamIdByName.Get(pTarget);

											switch(strgs.Get(1))
											{

												case "perm":
													mutedPlayersMap.Insert(mutedPlyrId, "perm");
													sendPlayerMessage(player, pTarget + " permanently muted.");
													if( mutedPlayer.GetIdentity().GetName() == pTarget)
														sendPlayerMessage(mutedPlayer, pTarget + " you have been muted by mods.");
													saveMutedPlayers();
													return;

												case "disable":
													mutedPlayersMap.Remove(mutedPlyrId);
													sendPlayerMessage(player, "Global chat enabled to " + pTarget + ".");
													if( mutedPlayer.GetIdentity().GetName() == pTarget)
														sendPlayerMessage(mutedPlayer, "Global chat enabled.");
													saveMutedPlayers();
													return;

												default:
													int muteTimeOff = strgs.Get(1).ToInt();
													if( muteTimeOff > 0)
													{
														mutedPlayersMap.Insert(mutedPlyrId, "" + muteTimeOff);
														GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(disablePlayerMute, muteTimeOff * 60 * 1000, false, mutedPlyrId);	
														sendPlayerMessage(player, pTarget + " will be muted for " + muteTimeOff + " minutes.");
														if( mutedPlayer.GetIdentity().GetName() == pTarget)
															sendPlayerMessage(mutedPlayer, pTarget + " You have been muted by mods.");
													}
													else
													{
														sendPlayerMessage(player, "Command not found. ¿Have you properly set the minutes?");
													}
													return;

											}
										}
										else{
											sendPlayerMessage(player, "Command not found, Is the player name correct? Name received: " + pTarget);
										}

									}
									return;
						}
						return;
					}
					return;
				}

				if (msg.Get(0) == "/")
				{

					switch (strgs.Get(0))
					{
						case "/chat":

							switch(strgs.Get(1))
							{

								case "local":
									if ( !isPlayerWithLocalChat(player) )
									{
										activePlayersConfig.Get(player.GetIdentity().GetId()).Set("localChat", "true");
										sendPlayerMessage(player, "Global chat disabled.");
									}
									return;

								case "global":
									if( isPlayerMuted(player) )
									{
										sendPlayerMessage(player, "You are muted from global chat.");
										return;
									}
									if ( isPlayerWithLocalChat(player) )
									{
										activePlayersConfig.Get(player.GetIdentity().GetId()).Set("localChat", "false");
										sendPlayerMessage(player, "Global chat enabled.");
									}
									return;
								}
							return;

						case "/killfeed":

							switch(strgs.Get(1))
							{

								case "off":
									if(!isPlayerWithKillFeedOn(player.GetIdentity().GetId()))
									{
										killFeedOnRequest.Remove(player.GetIdentity().GetId());
										sendPlayerMessage(player, "KillFeed already disabled.");
									}
									else{
										killFeedOnRequest.Remove(player.GetIdentity().GetId());
										killFeedOffRequest.Set(player.GetIdentity().GetId(), GetGame().GetTime());
										sendPlayerMessage(player, "KillFeed will be disabled in "+ sysConfig.Get("killFeedNotifyMinutes") +" minutos.");
									}
									return;

								case "on":
									if(isPlayerWithKillFeedOn(player.GetIdentity().GetId()))
									{
										killFeedOffRequest.Remove(player.GetIdentity().GetId());
										sendPlayerMessage(player, "KillFeed already enabled.");
									}
									else{
										killFeedOffRequest.Remove(player.GetIdentity().GetId());
										killFeedOnRequest.Set(player.GetIdentity().GetId(), GetGame().GetTime());
										sendPlayerMessage(player, "KillFeed will be enabled in "+ sysConfig.Get("killFeedNotifyMinutes") +" minutos.");
									}
									return;
							}
							return;

						case "/stats":
							
							int kills = activePlayersConfig.Get(player.GetIdentity().GetId()).Get("kills").ToInt();
							int deaths = activePlayersConfig.Get(player.GetIdentity().GetId()).Get("deaths").ToInt();
							string killFeedOnStr = activePlayersConfig.Get(player.GetIdentity().GetId()).Get("killFeedOn");

							if( killFeedOnStr == "true")
								killFeedOnStr = "Enabled";
							else
								killFeedOnStr = "Disabled";

							msg = "[[Stats]] Name: "+ player.GetIdentity().GetName() +" | Kills: "+ kills +" | Deaths: "+ deaths +" | K/D Ratio: "+ (1.0*kills)/(1.0*deaths) +" | KillFeed Status: "+ killFeedOnStr;
							
							sendPlayerMessage(player, msg);

							return;
						default:

							msg = "Command not found.";
							sendPlayerMessage(player, msg);

							return;

					}
					return;
				}

				if( isPlayerWithLocalChat(player) || isPlayerMuted(player) )
					return
				else{

					sendGlobalMessage(senderName + ": " + msg);
					return;
				}
		}
		
		// Unless chat command was executed, operate normally
		// Call super class event handler to handle other events
	}

	PlayerBase getPlayer(string tag)
	{
		ref array<Man> players = new array<Man>;
		GetGame().GetPlayers( players );
		
		PlayerBase p;
		
		bool nameMatch;
		if(!steamIdByName.Contains(tag)) 
			return NULL
		for ( int i = 0; i < players.Count(); ++i )
		{
			Class.CastTo(p, players.Get(i));
			
			// Store matches from different checks
			nameMatch = p.GetIdentity().GetId() == steamIdByName.Get(tag);
		
			if ( nameMatch )
				return p;
			
		}
		
		// Player with given parameter not found
		return NULL;
	}

	void tpFromToPlayer(PlayerBase mod, string plTPName)
	{
		if (!mod) return;
		if (!plTPName) return;
		
		PlayerBase playerTPed = getPlayer(plTPName);
		vector toPos = mod.GetPosition();

		float pos_x = toPos[0];
		float pos_z = toPos[2];
		float pos_y = GetGame().SurfaceY(pos_x, pos_z);
		
		vector pos = Vector(pos_x, pos_y, pos_z);
		
		playerTPed.SetPosition(pos);
	}

	void tpToCoord(string senderName, string coordX, string coordY)
	{
		string pos = coordX + " 0 " + coordY;

		PlayerBase player = getPlayer(senderName);
		// Safe conversion
		vector p = pos.ToVector();
		
		// Check that position is a valid coordinate
		// 0 0 0 wont be accepted even though valid
		if (p) {
			// Get safe surface value for Y coordinate in that position
			p[1] = GetGame().SurfaceY(p[0], p[2]);
			player.SetPosition(p);			
			return;
		}
		
	}
	bool isPlayerWithLocalChat(PlayerBase player)
	{
		return activePlayersConfig.Contains(player.GetIdentity().GetId()) && activePlayersConfig.Get(player.GetIdentity().GetId()).Get("localChat") == "true";
	}

	bool isPlayerMuted(PlayerBase player)
	{
		return mutedPlayersMap.Contains(player.GetIdentity().GetId());
	}

	bool isPlayerAdmin(string pAdId2)
	{
		return adminPlayersIds.Contains(pAdId2);
	}
	bool isPlayerMod(string pAdId2)
	{
		return modPlayersIds.Contains(pAdId2);
	}

	void sendGlobalMessage(string message)	
	{
		ref array<Man> players = new array<Man>;
		GetGame().GetPlayers( players );
		string playerName;
		
		for ( int i = 0; i < players.Count(); i++ )
		{
			Man player = players.Get(i);
			if ( player )
			{
				playerName = player.GetIdentity().GetName();
				if ( !isPlayerWithLocalChat(player) )
					sendPlayerMessage(player, message);
			}
		}
	}

	void sendPlayerMessage(PlayerBase player, string message)	
	{
		Param1<string> Msgparam;
		Msgparam = new Param1<string>(message);
		GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, player.GetIdentity());
	}

	string getDate()
	{
		int year;
		int month;
		int day;
		  
		GetYearMonthDay(year, month, day);
		string d = year.ToStringLen(2) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2);
		  
		return d;
	}
/*
CONTROL DE FECHAS 

	int fromYMDToDays(int y, int m, int d){
		m = (m + 9) % 12;
		y = y - m/10;

		return  365 * y + y / 4 - y / 100 + y / 400 + ( m * 306 + 5) / 10 + ( d - 1 );
	}

	int diffDays(string date1, string date2){

	    int y1 = date1.Substring(0, date1.IndexOf("-")).ToInt();
	    int y2 = date2.Substring(0, date2.IndexOf("-")).ToInt();
	
	    date1 = date1.Substring(date1.IndexOf("-") + 1, date1.Length());
	    date2 = date2.Substring(date2.IndexOf("-") + 1, date2.Length());
	    
	    int m1 = date1.Substring(0, date1.IndexOf("-")).ToInt();
	    int m2 = date2.Substring(0, date2.IndexOf("-")).ToInt();
	
	    date1 = date1.Substring(date1.IndexOf("-") + 1, date1.Length());
	    date2 = date2.Substring(date2.IndexOf("-") + 1, date2.Length());
	    
	    int d1 = date1.ToInt();
	    int d2 = date2.ToInt();

	    int days1 = fromYMDToDays(y1, m1, d1);
	    int days2 = fromYMDToDays(y2, m2, d2);

	    return days2 - days1;
	}
*/
	string getNameWithoutNumber(string name)
	{
		if(name.Length() > 4)
		{
			if ( name.Get(name.Length() - 3) == "(" && name.Get(name.Length() - 1) == ")" )
				return name.Substring(0, name.Length() - 4);
			if ( name.Get(name.Length() - 4) == "(" && name.Get(name.Length() - 1) == ")" )
				return name.Substring(0, name.Length() - 5);
		}	
		return name;
	}


	//name B must be the recent one
	bool sameName(string nameA, string nameB)
	{
		return nameA == getNameWithoutNumber(nameB);
	}

	void alertNameDuplicated(PlayerBase player)
	{
		sendPlayerMessage(player, "Name already registered. Please, use a diferent name, thank you. You will be kicked out shortly.");
	}

	void alertNameRemoved(PlayerBase player)
	{
		sendPlayerMessage(player, "You have removed your name in game, and you are named Survivor instead of " + activePlayersConfig.Get(player.GetIdentity().GetId()).Get("name") + ". Remember to change it if you want to use commands.");
	}

	void kickNameDuplicated(PlayerBase player)
	{
  		OnClientDisconnectedEvent(player.GetIdentity(), player, 10, false);
		//GetGame().DisconnectPlayer(playerIdentity);
	}


	void updateKillFeedConfig()
	{

		TStringArray idKFOArray = killFeedOnRequest.GetKeyArray();

		for (int kf1 = 0; kf1 < idKFOArray.Count(); kf1++){

			if(killFeedOnRequest.Contains(idKFOArray.Get(kf1)) && GetGame().GetTime() - killFeedOnRequest.Get(idKFOArray.Get(kf1)) < sysConfig.Get("killFeedNotifyMinutes") * 60 * 1000)
			{

				PlayerBase p = getPlayer(activePlayersConfig.Get(idKFOArray.Get(kf1)).Get("name"));
				if(sysConfig.Get("debug_enabled"))
					sendGlobalMessage("Changing killFeedOn for... " + idKFOArray.Get(kf1) + " with name " + activePlayersConfig.Get(idKFOArray.Get(kf1)).Get("name") + " and PlayerBaseObj "+ p);

				activePlayersConfig.Get(idKFOArray.Get(kf1)).Set("killFeedOn", "true");
				killFeedOnRequest.Remove(idKFOArray.Get(kf1));
				sendPlayerMessage(p, "KillFeed enabled. ");
			}

		}

		idKFOArray = killFeedOffRequest.GetKeyArray();
		
		for (kf1 = 0; kf1 < idKFOArray.Count(); kf1++){

			if(killFeedOffRequest.Contains(idKFOArray.Get(kf1)) && GetGame().GetTime() - killFeedOffRequest.Get(idKFOArray.Get(kf1)) < sysConfig.Get("killFeedNotifyMinutes") * 60 * 1000)
			{

				PlayerBase pl = getPlayer(activePlayersConfig.Get(idKFOArray.Get(kf1)).Get("name"));
				if(sysConfig.Get("debug_enabled"))
					sendGlobalMessage("Changing killFeedOff for... " + idKFOArray.Get(kf1) + " with name " + activePlayersConfig.Get(idKFOArray.Get(kf1)).Get("name") + " and PlayerBaseObj "+ p);

				activePlayersConfig.Get(idKFOArray.Get(kf1)).Set("killFeedOn", "false");
				killFeedOffRequest.Remove(idKFOArray.Get(kf1));
				sendPlayerMessage(pl, "KillFeed disabled. ");
			}

		}

	}

	void saveServerStatus()
	{
		if(sysConfig.Get("debug_enabled"))
			sendGlobalMessage("Saving...");
		savePlayersConfig();
		saveMutedPlayers();
	}

	void scheduleKillFeedLog(PlayerBase victimPBase)
	{

		bool canKillFeedBeLogged = false;
		string fLog = "";	
		string victimName = "";
		string killerName = "";
		
		if(!victimPBase.m_KillerData || !victimPBase.m_KillerData.m_Killer.IsPlayer())
			canKillFeedBeLogged = false;//isPlayerWithKillFeedOn(victimPBase);
		else
		{
			string victimId = victimPBase.GetIdentity().GetId();
			victimName = victimPBase.GetIdentity().GetName();

			Man killer = Man.Cast(victimPBase.m_KillerData.m_Killer);
			string killerId = killer.GetIdentity().GetId();
			killerName = killer.GetIdentity().GetName();
			
			if( victimId == killerId)
				canKillFeedBeLogged = false;//isPlayerWithKillFeedOn(victimPBase);
			else// if( activePlayersConfig.Contains(victimId) && activePlayersConfig.Contains(killerId) )
			{

				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(increaseKill, sysConfig.Get("killFeedNotifyMinutes") * 60 * 1000, false, killerId);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(increaseDeath, sysConfig.Get("killFeedNotifyMinutes") * 60 * 1000, false, victimId);
				
				canKillFeedBeLogged = isPlayerWithKillFeedOn(victimId) && isPlayerWithKillFeedOn(killerId);
			}
		}
		if(sysConfig.Get("debug_enabled"))
			sendGlobalMessage("Gonna Schedule FeedLog?..." + canKillFeedBeLogged);
		if(canKillFeedBeLogged && victimName + killerName != "" )
		{
		 			 	
		 	fLog = "[[KillFeed]] " + victimName + " was killed by " + killerName;

		 	if (victimPBase.m_KillerData.m_MurderWeapon.IsWeapon() || victimPBase.m_KillerData.m_MurderWeapon.IsMeleeWeapon()) 
		 	{
				fLog = fLog + " with " +  victimPBase.m_KillerData.m_MurderWeapon.GetDisplayName();
	            
	            //if (!victimPBase.m_KillerData.m_MurderWeapon.IsMeleeWeapon())
	            //    fLog = fLog + " desde " + vector.Distance(victimPBase.m_PlayerOldPos, killer.GetPosition()) + " meters";
			}
			//fLog = buildKillFeedMessage(victimPBase, victimPBase.m_KillerData.m_MurderWeapon);
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(sendGlobalMessage, sysConfig.Get("killFeedNotifyMinutes") * 60 * 1000, false, fLog);
		}
	}

	void increaseKill(string plId)	
	{
		activePlayersConfig.Get(plId).Set("kills", "" + (activePlayersConfig.Get(plId).Get("kills").ToInt() + 1) );		
	}

	void increaseDeath(string plId)	
	{
		activePlayersConfig.Get(plId).Set("deaths", "" + (activePlayersConfig.Get(plId).Get("deaths").ToInt() + 1) );

	}

	bool isPlayerWithKillFeedOn(string pLId)
	{	
		return activePlayersConfig.Get(pLId).Get("killFeedOn") == "true";
	}
		
	override void SyncRespawnModeInfo(PlayerIdentity identity)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_RespawnMode);
		rpc.Send(null, ERPCs.RPC_SERVER_RESPAWN_MODE, true, identity);
		scheduleKillFeedLog(getPlayer(identity.GetName()));
	}

	void disablePlayerMute(string pIdUnmute)
	{
		mutedPlayersMap.Remove(pIdUnmute);
	}
	

};

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}
