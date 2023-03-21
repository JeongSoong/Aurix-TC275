int Input[] = {0,0,0,0,0,0,0,0};
int Output[] = {0,0,0,0,0,0,0,0};

void setup() 
{ 
  size(1600,900); 
}
void draw() 
{ 
  background(65);
  fill(0,255,0);
  rect(100,100,1300,200);
  rect(100,500,1300,200);
  
  for(int i = 0; i < 8; i++)
  {
    fill(0);
    ellipse(200+i*150,200,80,80);
    ellipse(240+i*150,200,80,80);
    rect(200+i*150,160,40,80);  
    if(Input[i] == 0)
    {
      fill(255);
      ellipse(200+i*150,200,80,80);
    }
    else
    {
      fill(255);
      ellipse(240+i*150,200,80,80);
    }
      
      
    if (Input[i] == 0)
    {
    if(mousePressed && (mouseX > 160+i*150 && mouseX < 240 +i*150) &&(mouseY > 160 && mouseY < 240))
    {
       Input[i] = 1;
       delay(150);
    }
    }
   
   else
   {
    if(mousePressed && (mouseX > 220+i*150 && mouseX < 300 +i*150) &&(mouseY > 160 && mouseY < 240))
    {
      Input[i] = 0;
      delay(150);
    }
   }
   if(Input[i] == 0)
   {
     Output[i] = 0;
   }
   else
   {
     Output[i] = 1;
   }
   if(Output[i] == 0)
   {
     fill(255);
   }
   else
   {
     fill(255,0,0);
   }
   ellipse(200+i*150,600,80,80);
  }
}
