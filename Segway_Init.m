%Definitionen zum Segway State Space Control
%________________________________________________
%Segway:
l=0.45;  %Länge [m] Boden->Masseschwerpunkt
mp= 1.5; %Masse [kg] des ganzen Segways
g=9.81; %Erdbeschleunigung
Tp=1.6; %gemessene Schwingungsperiode [sek]

Jr=Tp^2*l*mp*g/(4*pi*pi); %Trägheitsmoment
Tm=0.15; %Motorzeitkonstante 
Km=0.163; %Motor-Streckenverstärkung [m/(sek*V)], für Kaskadenregler
Ta=1/100; %SampleTime (10msek)
n=1; %Cascade Controller (bei n=2,5 rutschen die Räder)
%_______________________________________________
% state variables s, v, phi, omega
A=[0         1         0          0;
   0        -1/Tm      0          0;
   0         0         0          1;
   0  -l*mp/(Jr*Tm)  l*mp*g/Jr  -0/Jr];

b=[ 0;
    1/Tm;
    0;
    1*l*mp/(Jr*Tm)];  %Km=1, da cascade controller

ct=[0 0 1 0];
%________________________________________________
%weighting matrix = Bestrafung
R=0.7 ; %Wichtung der Stellgröße; klein=schneller Regler
Q=[ 1  0  0  0;    %Weg
    0  0.1  0  0;  %Geschwindigkeit
    0  0  10  0;   %Winkel
    0  0  0  1];   %Winkelgeschwindigkeit
%________________________________________________
%LQ-Regler
K=lqr(A,b,Q,R);

%__________________________________________________
%Eigenwerte des geregelten Systems
Poles_R=eig(A-b*K);
%__________________________________________________
%Prefilter
cts=[1 0 0 0]; %mit Weg s(t) als Führungsgröße
F=1/(cts*(-A+b*K)^-1*b);

