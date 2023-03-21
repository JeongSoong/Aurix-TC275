boolean event = false;

void setup()
{
  size(1600, 900);
}
void draw()
{
  background(0);
  
  fill(0,255,0);
  rect(200,180,1200,150);
  rect(200,580,1200,150);
  
  fill(255,0,0);
  ellipse(275,255,100,100);
  ellipse(425,255,100,100);
  ellipse(575,255,100,100);
  ellipse(725,255,100,100);
  ellipse(875,255,100,100);
  ellipse(1025,255,100,100);
  ellipse(1175,255,100,100);
  ellipse(1325,255,100,100);
  fill(255,255,255);
  ellipse(275,655,100,100);
  ellipse(425,655,100,100);
  ellipse(575,655,100,100);
  ellipse(725,655,100,100);
  ellipse(875,655,100,100);
  ellipse(1025,655,100,100);
  ellipse(1175,655,100,100);
  ellipse(1325,655,100,100);
  
  for(int i=0; i<8; i++)
  {
    if(mousePressed && (mouseX >225+i*150 && mouseX <325+i*150) && (mouseY > 205 && mouseY < 305))
    {
      fill(255,0,0);
      event = true;
      ellipse(275+i*150,655,100,100);
    }
  }
}
