/*
 MFMOscilloFrame.cc
 Copyright Acquisition group, GANIL Caen, France
 */

#include <iostream>
#include <cmath>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
using namespace std;

#include "MFMOscilloFrame.h"

//_______________________________________________________________________________
MFMOscilloFrame::MFMOscilloFrame(int unitBlock_size, int dataSource,
		int frameType, int revision, int frameSize, int headerSize,
		int itemSize, int nItems) {
	SetPointers();
	fCountNbEventCard =NULL;
}
//_______________________________________________________________________________
MFMOscilloFrame::MFMOscilloFrame() {
	//	cout << "debug constructor of MFMOscilloFrame::MFMOscilloFrame()\n";
	fCountNbEventCard =NULL;
}
//_______________________________________________________________________________
MFMOscilloFrame::~MFMOscilloFrame() {
	//cout << "debug destructor of MFMOscilloFrame::()\n";
	if (fCountNbEventCard){
		delete [] fCountNbEventCard;
		fCountNbEventCard =NULL;
	}
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetAttributs(void * pt) {

	SetPointers(pt);
	MFMBasicFrame::SetAttributs(pt);
}
//_______________________________________________________________________________
void MFMOscilloFrame::OscilloGetParameters(int i,uint16_t *value) {
	OscilloGetParametersByItem((MFM_OscilloItem *) GetItem(i), value);
}
//_______________________________________________________________________________
void MFMOscilloFrame::OscilloGetParametersByItem(MFM_OscilloItem *item,uint16_t *value) {
	uint16_t tmp=item->Value;
	if (fLocalIsBigEndian!=fFrameIsBigEndian)
		SwapInt16(&tmp);
	*value = tmp;
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfig(uint16_t config){
  /// Set all Config
	((MFM_OscilloHeader*) pHeader)->EvtInfo.Config = config;
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfig(uint16_t on_off,uint16_t basetime,uint16_t trig,uint16_t signal,uint16_t channel){
  /// Set all Config
	uint16_t config;
	config =((on_off<<15)&MFM_CONFIG_ONOFF_MSK)+
			((basetime<<11)&MFM_CONFIG_TIMEBASE_MSK)+
			((trig<<8)&MFM_CONFIG_TRIG_MSK)+
			((signal<<5)&MFM_CONFIG_SIGNAL_MSK)+
			((channel)&MFM_CONFIG_IDXCHAN_MSK);
	 SetConfig(config);
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfigOnOff(uint16_t onoff){
  /// Set  Config ON or OFF ( 1 or 0) in config
	onoff=	onoff & MFM_CHANNELID_NUMBER_MSK;
((MFM_OscilloHeader*) pHeader)->EvtInfo.Config =
		(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config & (~MFM_CONFIG_ONOFF_MSK))
		| (onoff<<15);
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfigTimeBase(uint16_t time){
	/// Set Time base in config
	time = (time <<11)& MFM_CONFIG_TIMEBASE_MSK;
	((MFM_OscilloHeader*) pHeader)->EvtInfo.Config =
			(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config & (~MFM_CONFIG_TIMEBASE_MSK))
			| time;
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfigTrig(uint16_t trig){
	/// Set Tring in config
	((MFM_OscilloHeader*) pHeader)->EvtInfo.Config =
			(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config & (~MFM_CONFIG_TRIG_MSK))
			| ((trig<<8) & MFM_CONFIG_TRIG_MSK);
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfigSignal(uint16_t signal){
	/// set Signal in config
	((MFM_OscilloHeader*) pHeader)->EvtInfo.Config =
			(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config & (~MFM_CONFIG_SIGNAL_MSK))
			| ((signal<<5) & MFM_CONFIG_SIGNAL_MSK);
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetConfigChannelIdx(uint16_t idx){
	/// return Channel idx from config
	idx = idx & MFM_CHANNELID_NUMBER_MSK;
	((MFM_OscilloHeader*) pHeader)->EvtInfo.Config =
			(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config & (~MFM_CHANNELID_NUMBER_MSK))
			| idx ;
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfig() const {
	/// return all config word
uint16_t tmp=(((MFM_OscilloHeader*) pHeader)->EvtInfo.Config);
	if (fLocalIsBigEndian!=fFrameIsBigEndian)
		SwapInt16(&tmp);
	return  tmp;
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfigOnOff() const {
	/// Get ON / OFF  bit from config
	return ((GetConfig() & MFM_CONFIG_ONOFF_MSK)>>15);
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfigTimeBase() const {
	/// return Time Base formconfig

	return ((GetConfig() & MFM_CONFIG_TIMEBASE_MSK)>>11);
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfigTrig() const {
	/// return trigger from config
	return ((GetConfig() & MFM_CONFIG_TRIG_MSK)>>8);
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfigSignal() const {
	/// return Signal from config
	return ((GetConfig() & MFM_CONFIG_SIGNAL_MSK)>>5);
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetConfigChannelIdx() const {
	/// return ChannelIdx from config
	return ((GetConfig() & MFM_CONFIG_IDXCHAN_MSK));
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetChannelIdxNumber() const {
    /// return channel number(on/off,time base, trig, signal, channel idx)
	return (MFM_CHANNELID_NUMBER_MSK & GetChannelIdx());
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetChannelIdxBoard() const {
    /// return channel board index
	return ((MFM_CHANNELID_BOARD_MSK& GetChannelIdx())>>5);
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetBoardId() const {
         return GetChannelIdxBoard();
}
//_______________________________________________________________________________
uint16_t MFMOscilloFrame::GetChannelIdx() const {
    /// return all channel index  (board index + channel number)
    uint16_t tmp=(((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx);
	if (fLocalIsBigEndian!=fFrameIsBigEndian)
		SwapInt16(&tmp);
	return  tmp;
}

/*//_______________________________________________________________________________

void MFMOscilloFrame::SetChannelIdxNumber(uint16_t idx){
    /// return channel number shoulb be >=1 and <= 4
	((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx=
			(((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx & (~MFM_CHANNELID_NUMBER_MSK))
			| (idx & MFM_CHANNELID_NUMBER_MSK);
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetChannelIdxBoard(uint16_t idx){
    /// set channel number
	cout <<" idx "<<idx<<" channelidx "<<((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx<<"\n";
	((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx=
		(((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx & (~MFM_CHANNELID_BOARD_MSK))
			| ((idx & MFM_CHANNELID_BOARD_MSK)<<5);
}
*/
//_______________________________________________________________________________
void MFMOscilloFrame::SetChannelIdx(uint16_t idx){
	  /// Set channel number ( board index + channel number)
	((MFM_OscilloHeader*) pHeader)->EvtInfo.ChannelIdx = idx;
}
//_______________________________________________________________________________
void MFMOscilloFrame::SetChannelIdx(uint16_t idxch,uint16_t idxboard){
	  /// Set channel number ( board index + channel number)
	uint16_t idx;
	idx = ((idxboard<<5)&(MFM_CHANNELID_BOARD_MSK)) + (idxch&MFM_CHANNELID_NUMBER_MSK);
	SetChannelIdx(idx);
}
//_______________________________________________________________________________
void MFMOscilloFrame::OscilloSetParameters(int i,uint16_t value) {
	OscilloSetParametersByItem((MFM_OscilloItem *) GetItem(i), value);
}
//_______________________________________________________________________________
void MFMOscilloFrame::OscilloSetParametersByItem(MFM_OscilloItem *item, uint16_t value) {
	item->Value = value;
}

//_______________________________________________________________________________
void MFMOscilloFrame::InitStat() {
	MFMCommonFrame::InitStat();
	int i;
	fCountNbEventCard = new long long[65536];
	for ( i = 0;i<65536;i++){
		fCountNbEventCard[i]=0;
	}

}
//____________________________________________________________________
void MFMOscilloFrame::FillStat() {
	MFMCommonFrame::FillStat();
	uint16_t id ;
	id = GetChannelIdx();
	fCountNbEventCard[id]++;

}
//____________________________________________________________________
string  MFMOscilloFrame::GetStat(string info){

	string display("");
	stringstream ss("");
	ss << MFMCommonFrame::GetStat( info);
	int i, j; int total =0;

	for ( i=0;i<65536;i++ ){

		if (fCountNbEventCard[i]!=0){
			j =i;
			ss << "Card "<< (((j& MFM_CHANNELID_BOARD_MSK)>>5) );
			j =i;
			ss << " Cristal  "<< (j& MFM_CHANNELID_NUMBER_MSK);
			ss << " NbEvents = "<< fCountNbEventCard[i] <<"\n";
			total += fCountNbEventCard[i];
		}
	}
	ss<<"Total Oscillo Frames       = "<< total<<"\n";
	display = ss.str();
	return display;
}

//_______________________________________________________________________________
string MFMOscilloFrame::GetHeaderDisplay(char* infotext) {
/// Return a string containing infomation of MFM Header\n
	/// if infotext is not NULL replace the standart "MFM header" title
	stringstream ss;
	string display("");
	ss << MFMBasicFrame::GetHeaderDisplay(infotext);
	ss << "   Config  : Channel Number = " << GetConfigChannelIdx();
	ss << "  Signal = " << GetConfigSignal();
	ss << "  Trig = " << GetConfigTrig();
	ss << "  Basetime = " << GetConfigTimeBase();
	ss << "  On/Off = " << GetConfigOnOff();
	ss << "\n";
	ss << "   Channel : Channel Number = " << GetChannelIdxNumber();
	ss << "  Board Number = " <<GetChannelIdxBoard();
	ss << "\n";
	display = ss.str();
	return display;
}

//_______________________________________________________________________________
//void MFMOscilloFrame::MFMBasicFrame:FillDataWithRamdomValue(uint16_t channelIdx, uint16_t Config) {
void MFMOscilloFrame::FillDataWithRamdomValue(uint64_t timestamp,uint32_t eventnumber,int nbitem ){
	uint16_t boardid =116;
	uint16_t Config =1;
	uint16_t idxch =  1;
	SetChannelIdx(idxch,boardid);
	SetConfigChannelIdx(boardid);
	SetConfigOnOff(1);
	SetConfigTimeBase(15);
	SetConfigTrig(1);
	SetConfigSignal(2);

	uint16_t j = 0;
	int i = 0;
	if  (nbitem>0)
	for (i = 0; i < nbitem; i++) {
		OscilloSetParameters(i,j++);
	}
}
//____________________________________________________________________________
