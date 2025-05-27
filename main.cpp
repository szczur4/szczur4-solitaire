#define _XOPEN_SOURCE_EXTENDED
#include<bits/stdc++.h>
#include<ncursesw/ncurses.h>
using namespace std;
vector<pair<char,char>>reserve,reserveShown;
pair<char,char>stacks[7][19],height[7];
int endValues[4],tmp,playTime,moves,renderMode,actionId;
bool started,finished,difficulty;
struct action{
	pair<int,int>bfr,aft;
	bool chg;
};
struct info{
	int mvs,t;
	bool diff;
};
inline bool operator>(const info&a,const info&b){return a.t==b.t?(a.mvs==b.mvs?a.diff>b.diff:a.mvs>b.mvs):a.t>b.t;}
priority_queue<info,vector<info>,greater<>>stats;
fstream fileStream;
wstring cardOutline[]={L"╔══════╗",L"║      ║",L"║      ║",L"║      ║",L"║      ║",L"╚══════╝"},
cardEmpty[]={L"┌ ─  ─ ┐",L"|      |",L"|      |",L"|      |",L"|      |",L"└ ─  ─ ┘"},
cardHidden[]={L"╔══════╗",L"║░░░░░░║",L"║░░░░░░║",L"║░░░░░░║",L"║░░░░░░║",L"╚══════╝"},
cardType[]={L"♥",L"♠",L"♦",L"♣"};
pair selCard={-1,-1};
action actions[10];
chrono::time_point<chrono::steady_clock>timeAtStart,elapsed;
MEVENT mEvent;
///saves and reloads stats from a file
void reloadStats(){
	fileStream.open("stats.txt",ios::out);
	for(int i=0;i<10&&!stats.empty();i++){
		fileStream<<stats.top().mvs<<" "<<stats.top().t<<" "<<stats.top().diff<<endl;
		stats.pop();
	}
	fileStream.close();
	fileStream.open("stats.txt",ios::in);
	int a,b;
	bool c;
	for(int i=0;i<10&&fileStream>>a>>b>>c;i++)stats.emplace(a,b,c);
	fileStream.close();
}
///clears and initializes everything
void init(){
	finished=false;
	started=false;
	renderMode=2;
	moves=0;
	actionId=-1;
	initscr();
	curs_set(0);
	start_color();
	init_pair(1,COLOR_RED,COLOR_BLACK);
	init_pair(2,COLOR_GREEN,COLOR_BLACK);
	resize_term(40,125);
	noecho();
	raw();
	mouseinterval(0);
	keypad(stdscr,true);
	mousemask(ALL_MOUSE_EVENTS,nullptr);
	selCard={-1,-1};
	reserve.clear();
	reserveShown.clear();
	for(auto&stack:stacks)for(auto&card:stack)card={0,0};
	for(auto &i:height)i={-1,0};
}
///generates cards and starts the clock
void startGame(const bool level){
	difficulty=level;
	for(int y=0;y<4;y++){
		endValues[y]=0;
		for(int x=1;x<14;x++)reserve.emplace_back(x,y);
	}
	mt19937 rng(chrono::system_clock::now().time_since_epoch().count());
	ranges::shuffle(reserve,rng);
	for(int y=0;y<7;y++){
		for(int x=0;x<=y;x++){
			stacks[y][x]=reserve.back();
			reserve.pop_back();
		}
		height[y]={y-1,y};
	}
	renderMode=0;
	timeAtStart=chrono::steady_clock::now();
}
///draws an empty stack at given coordinates
void drawEmpty(const int x,const int y){
	attrset(COLOR_PAIR(2));
	for(int i=0;i<6;i++)mvaddwstr(i+y,x,cardEmpty[i].c_str());
}
///draws a hidden card at given coordinates
void drawHidden(const int x,const int y){
	attrset(COLOR_PAIR(2));
	for(int i=0;i<6;i++)mvaddwstr(i+y,x,cardHidden[i].c_str());
}
///draws a card with a given value and type at given coordinates
void drawCard(const int value,const int type,const bool shown,const bool hasSelected,const int cardId,int x,int y){
	hasSelected&&cardId>=selCard.second?attrset(COLOR_PAIR(0)):attrset(COLOR_PAIR(2));
	for(int i=0;i<6;i++)mvaddwstr(y+i,x,cardOutline[i].c_str());
	x++;
	y++;
	if(!shown){
		for(int i=0;i<4;i++)mvaddwstr(y+i,x-1,cardHidden[i+1].c_str());
		return;
	}
	attrset(COLOR_PAIR(!(type&1)));
	switch(value){
		case 0:drawEmpty(x-1,y-1);break;
		case 1:mvprintw(y,x+1,"A");mvprintw(y+3,x+1,"A");break;
		case 11:mvprintw(y,x+1,"J");mvprintw(y+3,x+1,"J");break;
		case 12:mvprintw(y,x+1,"Q");mvprintw(y+3,x+1,"Q");break;
		case 13:mvprintw(y,x+1,"K");mvprintw(y+3,x+1,"K");break;
		default:mvprintw(y,x+1,"%i",value);mvprintw(y+3,x+1,"%i",value);break;
	}
	if(value!=0){
		mvaddwstr(y,x+4,cardType[type].c_str());
		mvaddwstr(y+3,x+4,cardType[type].c_str());
	}
	attrset(COLOR_PAIR(2));
}
///draws reserve cards
void drawReserve(){
	reserve.empty()?drawEmpty(2,1):drawHidden(2,1);
	mvaddstr(2,11,"==>");
	mvaddstr(5,11,"==>");
	reserveShown.empty()?drawEmpty(15,1):drawCard(reserveShown.back().first,reserveShown.back().second,true,selCard.first=='r',0,15,1);
	attrset(COLOR_PAIR(2));
	mvprintw(7,5,"%llu",reserve.size());
	mvprintw(7,18,"%llu",reserveShown.size());
}
///draws the stacks where you put your card at the end
void drawEndStacks(){
	wstring tmp;
	for(int i=0;i<4;i++){
		drawCard(endValues[i],i,true,false,0,29+i*9,1);
		if(endValues[i]==0){
			attrset(COLOR_PAIR(i&1^1));
			tmp=cardType[i]+L"  "+cardType[i];
			mvaddwstr(2,31+i*9,tmp.c_str());
			mvaddwstr(5,31+i*9,tmp.c_str());
			attrset(COLOR_PAIR(2));
		}
	}
}
///draws given stats on given coordinates in the terminal
void drawStats(const int m,const int t,const bool d){
	attrset(COLOR_PAIR(0));
	printw("%i",m);
	attrset(COLOR_PAIR(2));
	printw(", Time: ");
	attrset(COLOR_PAIR(0));
	printw("%ih %im %gs",t/360000,t/6000%60,t%6000/100.0);
	attrset(COLOR_PAIR(2));
	printw(", Difficulty: ");
	attrset(COLOR_PAIR(0));
	printw("%s",d?"Hard":"Easy");
	attrset(COLOR_PAIR(2));
}
///main renderer
void repaint(){
	if(renderMode!=0)timeAtStart=chrono::steady_clock::now();
	if(!finished)playTime=static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-timeAtStart).count())/10;
	clear();
	for(char x=0;x<7;x++)for(char y=0;y<19&&(!stacks[x][y].first==0||y==0);y++)drawCard(stacks[x][y].first,stacks[x][y].second,y>height[x].first,x==selCard.first,y,2+x*9,8+(y<<1));
	drawReserve();
	drawEndStacks();
	for(int i=0;i<40;i++)mvaddwstr(i,66,L"│");
	for(int i=0;i<4;i++){
		if(endValues[i]>12){
			attrset(COLOR_PAIR(2));
			mvaddwstr(7,32+i*9,L"√");
		}
		else{
			attrset(COLOR_PAIR(1));
			mvaddch(7,32+i*9,'X');
		}
	}
	if(renderMode==2){
		attrset(COLOR_PAIR(2));
		mvaddwstr(12,18,L"╔══════════════════════════════╗");
		mvaddwstr(13,18,L"║    Select game difficulty    ║");
		mvaddwstr(14,18,L"║                              ║");
		mvaddwstr(15,18,L"║          E -> Easy           ║");
		mvaddwstr(16,18,L"║          H -> Hard           ║");
		mvaddwstr(17,18,L"║                              ║");
		mvaddwstr(18,18,L"╚══════════════════════════════╝");
	}
	else{
		renderMode=1;
		for(const int v:endValues)if(v<13)renderMode=0;
		if(renderMode&1){
			if(!finished)stats.emplace(moves,playTime,difficulty);
			reloadStats();
			mvaddwstr(12,18,L"╔══════════════════════════════╗");
			mvaddwstr(13,18,L"║           You won!           ║");
			mvaddwstr(14,18,L"║ Moves:                       ║");
			mvaddwstr(15,18,L"║ Time:                        ║");
			mvaddwstr(16,18,L"║ Difficulty:                  ║");
			mvaddwstr(17,18,L"║      Press r to restart      ║");
			mvaddwstr(18,18,L"╚══════════════════════════════╝");
			attrset(COLOR_PAIR(0));
			mvprintw(14,27,"%i",moves);
			mvprintw(15,26,"%ih %im %gs",playTime/360000,playTime/6000%60,playTime%6000/100.0);
			mvprintw(16,32,"%s",difficulty?"Hard":"Easy");
			finished=true;
		}
	}
	attrset(COLOR_PAIR(2));
	auto tmpQ(stats);
	mvprintw(1,69,"Current stats:");
	mvprintw(2,69,"Moves: ");
	drawStats(moves,playTime,difficulty);
	mvprintw(4,69,"Previous bests:");
	mvaddwstr(3,69,L"────────────────────────────────────────────────────────");
	for(int i=1;i<11;i++){
		if(!tmpQ.empty()){
			mvprintw(i+4,69,"%i. Moves: ",i);
			drawStats(tmpQ.top().mvs,tmpQ.top().t,tmpQ.top().diff);
			tmpQ.pop();
			continue;
		}
		mvprintw(i+4,69,"%i. Moves: ",i);
		attrset(COLOR_PAIR(0));
		addwstr(L"─");
		attrset(COLOR_PAIR(2));
		printw(", Time: ");
		attrset(COLOR_PAIR(0));
		addwstr(L"─");
		attrset(COLOR_PAIR(2));
	}
	mvaddwstr(15,69,L"────────────────────────────────────────────────────────");
	mvprintw(16,69,"How to play");
	mvprintw(18,69,"Press F to end current game");
	mvprintw(19,69,"Press R to restart on current difficulty");
	mvprintw(20,69,"Press ESC or Q to exit");
	mvprintw(21,69,"Press Z to undo a move (up to 10)");
	mvprintw(23,69,"Click on cards to select them");
	mvprintw(24,69,"Click another card to move the selected one");
	mvprintw(26,69,"Select a card to start counting the time");
	refresh();
}
///gets selected card location and returns a pair of values corresponding to it
pair<int,int>getCard(const int x,const int y){
	if(renderMode==0){
		if(!started){
			started=true;
			timeAtStart=chrono::steady_clock::now();
		}
		if(x>0&&x<9&&y>0&&y<7){
			for(int i=0;i<(difficulty?3:1);i++){
				if(reserve.empty()){
					mt19937 rng(chrono::system_clock::now().time_since_epoch().count());
					ranges::shuffle(reserveShown,rng);
					swap(reserve,reserveShown);
				}
				reserveShown.emplace_back(reserve.back());
				reserve.pop_back();
			}
			moves++;
			selCard={-1,-1};
		}
		else if(x>13&&x<22&&y>0&&y<7&&!reserveShown.empty())return{'r',0};
		else if(x>1&&x<64&&y>7)return{(x-2)/9,max(min(static_cast<char>(y-8>>1),height[(x-2)/9].second),static_cast<char>(height[(x-2)/9].first+1))};
		else if(x>29&&y>0&&y<7)return{'e',(x-29)/9};
		started=true;
	}
	return{-1,-1};
}
///undoes a move
void undo(const action&a){
	const int bfrX=a.bfr.first,bfrY=a.bfr.second,aftX=a.aft.first,aftY=a.aft.second;
	const bool chg=a.chg;
	if(bfrX=='r'){
		if(aftX=='e'){
			reserveShown.emplace_back(endValues[aftY],aftY);
			endValues[aftY]--;
		}
		else{
			reserveShown.emplace_back(stacks[aftX][height[aftX].second]);
			stacks[aftX][height[aftX].second--]={0,0};
		}
	}
	else{
		if(aftX=='e'){
			stacks[bfrX][bfrY]={endValues[aftY],aftY};
			endValues[aftY]--;
			height[bfrX].second++;
			if(chg)height[bfrX].first++;
		}
		else{
			const int n=height[aftX].second-aftY+1;
			for(int i=1;i<n;i++){
				stacks[bfrX][++height[bfrX].second]=stacks[aftX][aftY+i];
				stacks[aftX][aftY+i]={0,0};
				height[aftX].second--;
			}
			if(chg)height[bfrX].first++;
			height[aftX].first=min(height[aftX].first,static_cast<char>(height[aftX].second-1));;
		}
	}
	moves++;
	actionId--;
	actionId=max(actionId,-1);
}
///main logic
int main(){
	fileStream.open("stats.txt",ios::in);
	int a,b;
	bool c;
	for(int i=0;i<10&&fileStream>>a>>b>>c;i++)stats.emplace(a,b,c);
	fileStream.close();
	init();
	repaint();
	while(true){
		skip:switch(getch()){
			case 27:case'q':endwin();exit(0);
			case'r':if(renderMode!=2){
				init();
				startGame(difficulty);
			}
			break;
			case'f':init();break;
			case'e':if(renderMode==2){startGame(false);break;}goto skip;
			case'h':if(renderMode==2){startGame(true);break;}goto skip;
			case'z':if(renderMode==0&actionId>-1){undo(actions[actionId]);break;}goto skip;
			case KEY_MOUSE:if(getmouse(&mEvent)==OK){
				//checks if both cards selected can be placed on to of each other and places them if they can be
				const auto card=getCard(mEvent.x,mEvent.y);
				if(selCard.first>-1&&selCard.first<7){
					if(card.first=='e'){
						if(auto[v,t]=stacks[selCard.first][selCard.second];selCard.second==height[selCard.first].second&&t==card.second&&v==endValues[card.second]+1){
							endValues[card.second]++;
							stacks[selCard.first][selCard.second]={0,0};
							height[selCard.first].second--;
							moves++;
						}
						if(actionId==9)for(int i=0;i<9;)swap(actions[i],actions[++i]);
						else actionId++;
						actions[actionId]={selCard,card,height[selCard.first].first==height[selCard.first].second};
						height[selCard.first].first=min(height[selCard.first].first,static_cast<char>(height[selCard.first].second-1));
						selCard={-1,-1};
					}
					else if(card.first>-1&&card.first<7){
						if(stacks[selCard.first][selCard.second].first==stacks[card.first][height[card.first].second].first-1&&(stacks[selCard.first][selCard.second].second^stacks[card.first][height[card.first].second].second)&1||stacks[selCard.first][selCard.second].first==13&&stacks[card.first][card.second].first==0){
							const int n=height[selCard.first].second-selCard.second+1;
							for(int i=0;i<n;i++){
								stacks[card.first][++height[card.first].second]=stacks[selCard.first][selCard.second+i];
								stacks[selCard.first][selCard.second+i]={0,0};
								height[selCard.first].second--;
							}
							moves++;
							if(actionId==9)for(int i=0;i<9;)swap(actions[i],actions[++i]);
							else actionId++;
							actions[actionId]={selCard,card,height[selCard.first].first>=height[selCard.first].second};
							if(height[selCard.first].first>=height[selCard.first].second)height[selCard.first].first=static_cast<char>(height[selCard.first].second-1);
							selCard={-1,-1};
						}
						else selCard=card;
					}
					else selCard=card;
				}
				else if(selCard.first=='r'){
					if(card.first=='e'){
						if(auto[v,t]=reserveShown.back();v==endValues[card.second]+1&&t==card.second){
							endValues[card.second]++;
							reserveShown.pop_back();
							moves++;
						}
						if(actionId==9)for(int i=0;i<9;)swap(actions[i],actions[++i]);
						else actionId++;
						actions[actionId]={selCard,card,false};
						selCard={-1,-1};
					}
					else if(card.first>-1&&card.first<7){
						auto[v1,t1]=reserveShown.back();
						if(auto[v2,t2]=stacks[card.first][height[card.first].second];v1==v2-1&&(t1^t2)&1||v1==13&&v2==0){
							stacks[card.first][++height[card.first].second]=reserveShown.back();
							reserveShown.pop_back();
							if(actionId==9)for(int i=0;i<9;)swap(actions[i],actions[++i]);
							else actionId++;
							actions[actionId]={selCard,card,false};
							selCard={-1,-1};
							moves++;
						}
						else selCard=card;
					}
					else selCard=card;
				}
				else selCard=card;
				break;
			}
			default:goto skip;
		}
		repaint();
	}
}
