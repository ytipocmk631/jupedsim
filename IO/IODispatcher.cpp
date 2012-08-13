/**
 * File:   IODispatcher.cpp
 *
 * Created on 20. November 2010, 15:20
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "IODispatcher.h"


string IODispatcher::WritePed(Pedestrian* ped) {
	double v, a, b, phi;
	double RAD2DEG = 180.0 / M_PI;
	char tmp[CLENGTH] = "";

	v = ped->GetV().Norm();
	int color;
	double v0 = ped->GetV0Norm();
	if (v0 == 0) {
		Log->write("ERROR: IODispatcher::WritePed()\t v0=0");
		exit(0);
	}
	color = (int) (v / v0 * 255);
	a = ped->GetLargerAxis();
	b = ped->GetSmallerAxis();
	phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
	sprintf(tmp, "<agent ID=\"%d\"\t"
			"xPos=\"%.2f\"\tyPos=\"%.2f\""
			"radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
			"ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
			ped->GetPedIndex(), (ped->GetPos().GetX()) * FAKTOR,
			(ped->GetPos().GetY()) * FAKTOR,
			a*FAKTOR, b*FAKTOR, phi * RAD2DEG, color);
	return tmp;
}


// Konstruktoren

IODispatcher::IODispatcher() {
	pHandlers = vector<OutputHandler* > ();
}

IODispatcher::IODispatcher(const IODispatcher& orig) {
}

IODispatcher::~IODispatcher() {
	for (int i = 0; i < (int) pHandlers.size(); i++)
		delete pHandlers[i];
	pHandlers.clear();
}

void IODispatcher::AddIO(OutputHandler* ioh) {
	pHandlers.push_back(ioh);
};

const vector<OutputHandler*>& IODispatcher::GetIOHandlers() {
	return pHandlers;
}

void IODispatcher::Write(string str) {
	for (vector<OutputHandler* >::iterator it = pHandlers.begin(); it != pHandlers.end(); ++it) {
		(*it)->write(str);
	}

};

void IODispatcher::WriteHeader(int nPeds, int fps, Building* building, int seed, int szenarioID) {

	nPeds=building->GetAnzPedestrians();
	string tmp;
	tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n""<trajectoriesDataset>\n";
	tmp.append("\t<header formatVersion = \"1.0\">\n");
	char agents[50] = "";
	sprintf(agents, "\t\t<agents>%d</agents>\n", nPeds);
	tmp.append(agents);
	sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
	tmp.append(agents);
	sprintf(agents, "\t\t<frameRate>%d</frameRate>\n", fps);
	tmp.append(agents);
	tmp.append("\t</header>\n");
	Write(tmp);

}

void IODispatcher::WriteGeometry(Building* building) {
	string geometry;
	geometry.append("\t<geometry>\n");

	bool plotHlines=true;
	bool plotCrossings=true;
	bool plotTransitions=true;
	vector<string> rooms_to_plot;

	//Promenade
	//rooms_to_plot.push_back("outside");
	//rooms_to_plot.push_back("010");

	// first the rooms
	for (int i = 0; i < building->GetAnzRooms(); i++) {
		Room* r = building->GetRoom(i);
		string caption=r->GetCaption();
		if(rooms_to_plot.empty()==false)
			if(IsElementInVector(rooms_to_plot,caption)==false) continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			geometry.append(s->WriteSubRoom());
		}
	}

	for (int i = 0; i < building->GetRouting()->GetAnzGoals(); i++) {
		Crossing* goal = building->GetRouting()->GetGoal(i);
		Room* room1 = goal->GetRoom1();
		string caption=room1->GetCaption();

		if(plotCrossings){
			if(goal->IsTransition()==false)
				if(goal->GetSubRoom1()!=goal->GetSubRoom2())
					if(rooms_to_plot.empty() || IsElementInVector(rooms_to_plot,caption)){
						geometry.append(goal->WriteElement());
					}
		}

		if (plotHlines){
			if(goal->IsTransition()==false)
				if(goal->GetSubRoom1()==goal->GetSubRoom2())
					if(rooms_to_plot.empty() || IsElementInVector(rooms_to_plot,caption)){
						geometry.append(goal->WriteElement());
					}
		}

		if (plotTransitions){
			if(goal->IsTransition()){
				if (rooms_to_plot.empty()){
					geometry.append(goal->WriteElement());

				} else {

					Transition* tr = (Transition*)(goal);
					Room* room1 = tr->GetRoom1();
					Room* room2 = tr->GetRoom2();
					string caption1=room1->GetCaption();

					if(room2){
						string caption2=room2->GetCaption();
						if(IsElementInVector(rooms_to_plot,caption1) ||
								IsElementInVector(rooms_to_plot,caption2)){
							geometry.append(goal->WriteElement());
						}

					}else{
						if(IsElementInVector(rooms_to_plot,caption1)){
							geometry.append(goal->WriteElement());
						}
					}
				}
			}
		}
	}


	geometry.append("\t</geometry>\n");
	Write(geometry);
}

void IODispatcher::WriteFrame(int frameNr, Building* building) {
	string data;
	char tmp[CLENGTH] = "";
	vector<string> rooms_to_plot;

	//promenade
	//rooms_to_plot.push_back("010");

	sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
	data.append(tmp);


	for (int roomindex = 0; roomindex < building->GetAnzRooms(); roomindex++) {
		Room* r = building->GetRoom(roomindex);
		string caption = r->GetCaption();

		if((rooms_to_plot.empty()==false)&&(IsElementInVector(rooms_to_plot, caption)==false)){
			continue;
		}

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			for (int i = 0; i < s->GetAnzPedestrians(); ++i) {
				Pedestrian* ped = s->GetPedestrian(i);
				data.append(WritePed(ped));
			}
		}
	}
	data.append("</frame>\n");
	Write(data);
}

void IODispatcher::WriteFooter() {
	Write("</trajectoriesDataset>\n");
}


Trajectories::Trajectories() : IODispatcher() {

}

void Trajectories::WriteHeader(int nPeds, int fps, Building* building) {

}

void Trajectories::WriteGeometry(Building* building) {

}

void Trajectories::WriteFrame(int frameNr, Building* building) {
	char tmp[CLENGTH] = "";

	for (int roomindex = 0; roomindex < building->GetAnzRooms(); roomindex++) {
		Room* r = building->GetRoom(roomindex);
		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			for (int i = 0; i < s->GetAnzPedestrians(); ++i) {
				Pedestrian* ped = s->GetPedestrian(i);
				double x = ped->GetPos().GetX();
				double y = ped->GetPos().GetY();
				sprintf(tmp, "%d\t%d\t%f\t%f", ped->GetPedIndex(), frameNr, x, y);
				Write(tmp);
			}
		}
	}
}

void Trajectories::WriteFooter() {

}