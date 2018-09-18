#include "shape.h"

const double shp_vec_x[16] = {1, 1, 1, 0.5, 0, -0.5, -1, -1, -1, -1, -1, -0.5, 0, 0.5, 1, 1};
const double shp_vec_y[16] = {0, 0.5, 1, 1, 1, 1, 1, 0.5, 0, -0.5, -1, -1, -1, -1, -1, -0.5};
const double shp_arc_x[8] = {1, 0.707106781, 0, -0.707106781, -1, -0.707106781, 0, 0.707106781};	
const double shp_arc_y[8] = {0, 0.707106781, 1, 0.707106781, 0, -0.707106781, -1, -0.707106781};

void shp_font_add(shape *shp_font, long num, char *name, unsigned char *cmds, unsigned int cmd_size){
	if (shp_font){
		shape *new_font = (shape *) malloc(sizeof(shape));
		if (new_font){
			new_font->num = num;
			
			if (name){
				new_font->name = malloc(strlen(name)+1);
				strcpy(new_font->name, name);
			} else new_font->name = NULL;
			if (cmds){
				new_font->cmds = calloc(cmd_size, sizeof(unsigned char));
				memcpy(new_font->cmds, cmds, cmd_size);
			} else new_font->cmds = NULL;
			
			new_font->cmd_size = cmd_size;
			new_font->unicode = 1;
			
			new_font->next = NULL;
			
			
			if(shp_font->next == NULL){ //verifica se a lista esta vazia
				shp_font->next=new_font;} // o novo elemento e o ultimo
			else{ // lista nao vazia
				//busca o final da lista
				shape *tmp = shp_font->next;
				while(tmp->next != NULL){
					tmp = tmp->next;}
				tmp->next = new_font; //acrescenta o novo elemento no final
			}
		}
	}
}

void shp_font_free(shape *shp_font){
	if (shp_font){
		if(shp_font->next){ //verifica se a lista esta vazia
			shape *next_shp, *current;
			
			current = shp_font->next;
			shp_font->next = NULL;
			while(current){
				next_shp = current->next;
				free(current->name);
				free(current->cmds);
				free(current);
				current = next_shp;
			}
		}
	}
}

shape *shp_font_find(shape *shp_font, long num){
	if (shp_font){ //verifica se existe a shp_font
		if(shp_font->next){ //verifica se a lista esta vazia
			//pula o primeiro shape da shp_font que eh a descricao da propria shp_font
			if(shp_font->next->next){
				shape *current;
				current = shp_font->next->next;
				while(current){
					if(current->num == num){
						return(current);}
					current = current->next;
				}
			}
		}
	}
	return(NULL);
}

shape *shp_idx(shape *shp_font, int idx){
	if (shp_font){ //verifica se existe a shp_font
		if(shp_font->next){ //verifica se a lista esta vazia
			//pula o primeiro shape da shp_font que eh a descricao da propria shp_font
			//if(shp_font->next->next){
				shape *current;
				int pos = 0;
				current = shp_font->next;
				while(current){
					if(pos == idx){
						return(current);}
					current = current->next;
					pos ++;
				}
			//}
		}
	}
	return(NULL);
}
shape *shp_font_open(char *path){
	FILE *file;
	shape *shp_font;
	shape *curr_shp;
	
	int curr; /* current readed byte in file */
	long index = 0; /* current position in file */
	long next_index = 0; /* next mark to be read*/
	
	int file_start = 0; /* flag - indicates start of file readed*/
	int file_head = 0;
	int head = 0; /* flag - indicates head of shape readed*/
	int name = 0; /* flag - indicates data of shape readed*/
	enum shp_file_type type = SHP_NONE;
	
	int num; /* shape number */
	char str_tmp[255];
	unsigned char buffer[255];
	unsigned int buf_size = 0;
	
	int first_cp, last_cp, num_cp = 0, cp_idx = 0;
	int id_cp, bytes_cp;
	
	/* try to open file */
	file = fopen(path,"rb");
	if(file == NULL) {/*error */
		return(NULL);
	}
	
	/* create list of shapes*/
	shp_font = (shape *) malloc(sizeof(shape));
	if(shp_font == NULL) { /*error */
		fclose(file);
		return(NULL);
	}
	shp_font->next = NULL; /* empty list */
	
	buf_size = 0;
	while ((curr = (unsigned) fgetc(file)) != EOF){ /* read current position as unsigned int */
		index++;
		if (!file_start){
			/* start of file */
			if (curr != 26){
				if (buf_size < 254){
					str_tmp[buf_size] = (signed char) curr;
					buf_size++;
				}
				else break; /* error */
				continue;
			}
			else{ /* stop when find 26 (1A hex) byte in file */
				file_start = 1;
				
				str_tmp[buf_size] = 0; /* terminate string */
				/* identify type of file */
				if (strstr(str_tmp, "shapes")) type = SHP_SHAPES;
				else if (strstr(str_tmp, "unifont")) type = SHP_UNIFONT;
				else if (strstr(str_tmp, "bigfont")) type = SHP_BIGFONT;
					
				if (!((type == SHP_SHAPES) || (type == SHP_UNIFONT))) break;
				/* next field have 6 bytes long */
				next_index = index + 6;
				
				head = 0;
				file_head = 0;
				name = 0;
				buf_size = 0;
				continue;
			}
		}
		else if(type == SHP_SHAPES){
			if (!file_head){
				/* header for shapes files */
				if (index != next_index){
					if (buf_size < 255){
						buffer[buf_size] = curr;
						buf_size++;
					}
				}
				else {
					/* first code point in file */
					first_cp = ((buffer[1]&255) << 8)|(buffer[0]&255);
					/* last code point in file */
					last_cp = ((buffer[3]&255) << 8)|(buffer[2]&255);
					/* number of glyphs in file */
					num_cp = ((buffer[5]&255) << 8)|(buffer[4]&255);
					/* go to next step*/
					file_head = 1;
					head = 0;
					buf_size = 0;
					cp_idx = 0;
					next_index = index + 4;
				}
			}
			else if (!head){
				if (cp_idx < num_cp){
					/* read list of code point id */
					if (index != next_index){
						if (buf_size < 255){
							buffer[buf_size] = curr;
							buf_size++;
						}
					}
					else {
						if (buf_size < 255){
							buffer[buf_size] = curr;
							buf_size++;
						}
						/* code point */
						id_cp = ((buffer[1]&255) << 8)|(buffer[0]&255);
						/* data chunck size */
						bytes_cp = ((buffer[3]&255) << 8)|(buffer[2]&255);
						/* add shape id in list */
						shp_font_add(shp_font, id_cp, NULL, NULL, bytes_cp);
						
						/* go to next step */
						buf_size = 0;
						next_index = index + 4;
						cp_idx++;
					}
				}
				if (cp_idx >= num_cp) { /* end of list */
					head = 1;
					buf_size = 0;
					name = 0;
					/* prepare for read data of first code point */
					cp_idx = 0;
					curr_shp = shp_idx(shp_font, cp_idx);
					if (curr_shp){
						/* go to next step */
						next_index = index + curr_shp->cmd_size;
					}
					else break; /* error */
				}
			}
			else if (cp_idx < num_cp){
				if (index != next_index){
					if (!name){ /* get name and comments of glyph */
						if (curr != 0){ /* until string end (0x00) */
							if (buf_size < 255){
								str_tmp[buf_size] = curr;
								buf_size++;
							}
						}
						else{
							str_tmp[buf_size] = curr; /* terminate string */
							
							/* next, get shape commands */
							buf_size = 0;
							name = 1;
						}
					}
					else{ /* shape commands */
						if (buf_size < 255){
							buffer[buf_size] = curr;
							buf_size++;
						}
					}
				}
				else{
					if (curr != 0) break; /* error */
					/* get shape structure for current code point */
					curr_shp = shp_idx(shp_font, cp_idx);
					if (curr_shp){
						/* save name */
						curr_shp->name = malloc(strlen(str_tmp)+1);
						strcpy(curr_shp->name, str_tmp);
						/* save commands */
						curr_shp->cmds = malloc(buf_size);
						memcpy(curr_shp->cmds , buffer, buf_size);
						
						curr_shp->cmd_size = buf_size;
						curr_shp->unicode = 0;
					}
					/* go to next code point */
					cp_idx ++;
					curr_shp = shp_idx(shp_font, cp_idx);
					if (curr_shp){
						next_index = index + curr_shp->cmd_size;
					}
					else break;
					buf_size = 0;
					name = 0;
				}
				
			}
		}
		else if(type == SHP_UNIFONT){
			
			//a partir deste ponto comeca a ler as definicoes de cada letra
			// as rotinas abaixo serao repetidas para cada letra
			if (!head){
				//busca o cabecalho da letra
				if (index != next_index){
					buffer[buf_size] = curr; //armazena no buffer
					buf_size++;
				}
				else{ //termina quando atinge o comprimento de 4 bytes (ou 6, se for a definicao da shp_font)
					//verifica se o ultimo byte eh igual a zero, senao para por erro
					if (curr != 0){
						break; //<=erro
					}
					
					//o ultimo byte eh o comprimento do campo de dados da letra
					next_index = index + buffer[buf_size - 1];
					
					// combina os dois primeiros bytes como um inteiro, que eh a especificacao unicode
					num = ((buffer[1]&255) << 8)|(buffer[0]&255);
					
					//prepara para a leitura dos dados
					head = 1;
					name = 0;
					buf_size = 0;
				}
			}	
			else if (head){
				//busca os dados da letra
				if (index != next_index){
					if (!name){ //busca o name do shape
						if (curr != 0){ //o valor 00 indica o fim do name
							str_tmp[buf_size] = curr; //armazena na string temporaria
							buf_size++;
						}
						else{
							str_tmp[buf_size] = curr; // completa a string
							
							//prepara para leitura dos cmds de desenho
							buf_size = 0;
							name = 1;
						}
					}
					else{ //cmds de desenho
						buffer[buf_size] = curr; //armazena no buffer
						buf_size++;
					}
				}
				else{ //termina quando atinge o comprimento especificado
					//verifica se o ultimo byte eh igual a zero, senao para por erro
					if (curr != 0){
						break; //<=erro
					}
					//adiciona o shape a lista da shp_font
					shp_font_add(shp_font, num, str_tmp, buffer, buf_size);
					
					// prepara para a nextima letra
					next_index = index + 4; //cabecalho com 4 bytes de compr
					head = 0;
					buf_size = 0;
				}
			}
		}
	}
	//printf ("\nbytes: %d\n", index);
	
	fclose(file); // fecha o arquivo	
	return(shp_font); // retorna a shp_font
}

graph_obj *shp_parse_cp(shape *shp_font, int pool_idx, int cp, double *w){
	double pre_x = 0;
	double pre_y = 0;
	double px = 0;
	double py = 0;
	int pen = 1;
	graph_obj * line_list = NULL;
	/*
	double max_x = 0;
	double max_y = 0;
	double min_x = 0;
	double min_y = 0;
	*/
	double stack_x[50];
	double stack_y[50];
	int stk_size = 0;
	
	long index = 0;
	int cmd = 0; //sem cmd
	int bypass = 0;
	double bulge = 0;
	double scale = 1.0;
	double radius;
	double theta, alfa, d, ang_c, ang;

	double length, center_x, center_y, ang_ini;
	int vector, octant, direction, num_oct;

	
	int i;
	
	//cria a lista de retorno
	line_list = graph_new(pool_idx);
		
	shape *shp = shp_font_find(shp_font, (long) cp);
	if(shp){
		//printf("\nCod. %d\n", shp->num);
	//}
	//else: i = [42, 'asterisco', [0, 2, 14, 8, 254, 251, 33, 1, 68, 2, 46, 1, 72, 2, 65, 1, 74, 2, 68, 1, 78, 2, 47, 14, 8, 252, 253]]
	
	while (index < shp->cmd_size){
		cmd = shp->cmds[index];
		
		if(cmd > 14){
			/* imediate movement command */
			if(bypass) bypass = 0;
			else{
				/* vector length in high nibble */
				length = (cmd & 240)/16;
				/* vector index in lower nibble */
				vector = cmd & 15;
				/* direction in table */
				px = length * shp_vec_x[vector];
				py = length * shp_vec_y[vector];
				
				px *= scale;
				py *= scale;
				if(pen) /* add line */
					line_add(line_list, pre_x, pre_y, 0.0, pre_x+px, pre_y+py, 0.0);
				pre_x += px;
				pre_y += py;
			}
			index++;
		}
		else if(cmd == 1){
			/* draw activated */
			if (bypass) bypass = 0;
			else pen = 1;
			index++;
		}
		else if(cmd == 2){
			/* draw desactivated */
			if (bypass) bypass = 0;
			else pen = 0;
			index++;
		}
		else if(cmd == 3){
			/* divide factor */
			if(bypass) bypass = 0;
			else{
				if (abs(shp->cmds[index + 1]) > 0) 
					scale /= shp->cmds[index + 1];
			}
			index += 2;
		}
		else if(cmd == 4){
			/* multiply factor */
			if(bypass) bypass = 0;
			else{
				if (abs(shp->cmds[index + 1]) > 0) 
					scale *= shp->cmds[index + 1];
			}
			index += 2;
		}
		else if(cmd == 5){
			/* salve current position in stack */
			if(bypass) bypass = 0;
			else{
				stack_x[stk_size] = pre_x;
				stack_y[stk_size] = pre_y;
				stk_size++;
			}
			index++;
		}
		else if(cmd == 6){
			/* restore position from stack */
			if(bypass) bypass = 0;
			else if(stk_size>0){
				stk_size--;
				pre_x = stack_x[stk_size];
				pre_y = stack_y[stk_size];
			}
			index++;
		}
		else if(cmd == 7){
			/* subshape */
			if(bypass) bypass = 0;
			else{
				/*TODO*/
			}
			if (shp_font->unicode) index += 3;
			else index += 2;
		}
		else if(cmd == 8){
			/* one (x,y) coordinate */
			if(bypass) bypass = 0;
			else{
				px = (double)((signed char) shp->cmds[index + 1]);
				py = (double)((signed char) shp->cmds[index + 2]);
				px *= scale;
				py *= scale;
				if(pen) /* add line */
					line_add(line_list, pre_x, pre_y, 0.0, pre_x+px, pre_y+py, 0.0);
				pre_x += px;
				pre_y += py;
			}
			index += 3;
		}
		else if(cmd == 9){
			/* sequence of (x,y) coordinates, ended by (0,0) */
			index ++;
			while (!((shp->cmds[index] == 0) && (shp->cmds[index + 1] == 0))){
				if(!bypass){
					px = (double)((signed char) shp->cmds[index]);
					py = (double)((signed char) shp->cmds[index + 1]);
					px *= scale;
					py *= scale;
					if(pen) /* add line */
						line_add(line_list, pre_x, pre_y, 0.0, pre_x+px, pre_y+py, 0.0);
					pre_x += px;
					pre_y += py;
				}
				index += 2;
			}
			index += 2;
			
			if(bypass) bypass = 0;
			
		}
		else if(cmd == 10){
			/* octant arc */
			if(bypass) bypass = 0;
			else{
				radius = shp->cmds[index + 1] * scale;
				
				/* get first octant and direction from high nibble*/
				octant = (shp->cmds[index + 2] & 112)/16;
				direction = (shp->cmds[index + 2] & 128)/16;
				if(direction) direction = -1;
				else direction =1;
				
				/* get octant number from low nibble*/
				num_oct = shp->cmds[index + 2] & 15;
				if(num_oct == 0) num_oct = 8; /* full circle */
				
				center_x = pre_x - radius * shp_arc_x[octant];
				center_y = pre_y - radius * shp_arc_y[octant];
				ang_ini = octant * M_PI/4;
				for(i=1; i <= num_oct * 5; i++){ /* 5 samples for each octant */
					px = center_x + radius * cos(2 * M_PI * i * direction/ 40.0 + ang_ini);
					py = center_y + radius * sin(2 * M_PI * i * direction/ 40.0 + ang_ini);
					line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
					pre_x=px;
					pre_y=py;
				}
			
			}
			index += 3;
		}
		else if(cmd == 11){
			/* fractionary octant arc */
			if(bypass) bypass = 0;
			else{
				/*TODO*/
			}
			index += 6;
		}
		else if(cmd == 12){
			/* bulge arc */
			if(bypass) bypass = 0;
			else{
				px = (double)((signed char) shp->cmds[index + 1]);
				py = (double)((signed char) shp->cmds[index + 2]);
				bulge = (double)((signed char) shp->cmds[index + 3])/127.0;
				px *= scale;
				py *= scale;
				px += pre_x;
				py += pre_y;
				
				/* get radius and center of arc */
				
				theta = 2 * atan(bulge);
				alfa = atan2(py-pre_y, px-pre_x);
				d = sqrt((py-pre_y)*(py-pre_y) + (px-pre_x)*(px-pre_x)) / 2;
				radius = d*(bulge*bulge + 1)/(2*bulge);
				
				ang_c = M_PI+(alfa - M_PI/2 - theta);
				center_x = radius*cos(ang_c) + pre_x;
				center_y = radius*sin(ang_c) + pre_y;
				
				/* start and end angles obtained from points */
				ang_ini = atan2(pre_y-center_y,pre_x-center_x);
				double ang_end = atan2(py-center_y,px-center_x);
				direction = 1;
				if (bulge < 0){
					ang_ini += M_PI;
					ang_end += M_PI;
					direction = -1;
				}
				
				double ang = (ang_end - ang_ini) * direction; /* total angle */
				if (ang <= 0){ ang = ang + 2*M_PI;}
				
				/* sample arc in 64 points for full circle */
				int steps = (int) 64.0 * ang/(2 * M_PI);
				
				direction = 1;
				if (bulge < 0){
					direction = -1;
				}
				
				double final_x = px;
				double final_y = py;
				/* do arc */
				for(i=1; i < steps; i++){
					px = center_x + radius * cos(2/64.0 * M_PI * i * direction + ang_ini);
					py = center_y + radius * sin(2/64.0 * M_PI * i * direction + ang_ini);
					if(pen) line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
					pre_x=px;
					pre_y=py;
				}
				/* last point */
				if(pen) line_add(line_list, pre_x, pre_y, 0.0, final_x, final_y, 0.0);
				pre_x = final_x;
				pre_y = final_y;
			}
			index += 4;
		}
		else if(cmd == 13){
			/* sequence of bulge arcs, ended by (0,0) */
			
			index ++;
			while (!((shp->cmds[index] == 0) && (shp->cmds[index + 1] == 0))){
				if(!bypass){
					px = (double)((signed char) shp->cmds[index]);
					py = (double)((signed char) shp->cmds[index + 1]);
					bulge = (double)((signed char) shp->cmds[index + 2])/127.0;
					px *= scale;
					py *= scale;
					px += pre_x;
					py += pre_y;
					
					/* get radius and center of arc */
					
					theta = 2 * atan(bulge);
					alfa = atan2(py-pre_y, px-pre_x);
					d = sqrt((py-pre_y)*(py-pre_y) + (px-pre_x)*(px-pre_x)) / 2;
					radius = d*(bulge*bulge + 1)/(2*bulge);
					
					ang_c = M_PI+(alfa - M_PI/2 - theta);
					center_x = radius*cos(ang_c) + pre_x;
					center_y = radius*sin(ang_c) + pre_y;
					
					/* start and end angles obtained from points */
					ang_ini = atan2(pre_y-center_y,pre_x-center_x);
					double ang_end = atan2(py-center_y,px-center_x);
					direction = 1;
					if (bulge < 0){
						ang_ini += M_PI;
						ang_end += M_PI;
						direction = -1;
					}
					
					double ang = (ang_end - ang_ini) * direction; /* total angle */
					if (ang <= 0){ ang = ang + 2*M_PI;}
					
					/* sample arc in 64 points for full circle */
					int steps = (int) 64.0 * ang/(2 * M_PI);
					
					direction = 1;
					if (bulge < 0){
						direction = -1;
					}
					
					double final_x = px;
					double final_y = py;
					/* do arc */
					for(i=1; i < steps; i++){
						px = center_x + radius * cos(2/64.0 * M_PI * i * direction + ang_ini);
						py = center_y + radius * sin(2/64.0 * M_PI * i * direction + ang_ini);
						if(pen) line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
						pre_x=px;
						pre_y=py;
					}
					/* last point */
					if(pen) line_add(line_list, pre_x, pre_y, 0.0, final_x, final_y, 0.0);
					pre_x = final_x;
					pre_y = final_y;
				}
				index += 3;
			}
			index += 2;
			
			if(bypass) bypass = 0;
		}
		else if(cmd == 14){
			/* bypass next command, if is a vertical text */
			bypass = 1;
			index++;
			continue;
		}
		
	}
	}//temporario ate implementar o else
	
	
	if (w != NULL) *w = pre_x;
	return(line_list);
}

int shp_parse_str(shape *font, list_node *list_ret, int pool_idx, char *txt, double *w){
/* parse full string to graph*/
	if (!font || !list_ret || !txt) return 0;
	
	int ofs = 0, str_start = 0, code_p, num_graph = 0;
	graph_obj *curr_graph = NULL;
	
	double ofs_x = 0.0, width = 0.0;
	double fnt_above, fnt_below, fnt_size, txt_size = 1.0;
	
	/* find the dimentions of SHX font */
	if(font->next){ /* the font descriptor is stored in first iten of list */
		if(font->next->cmd_size > 1){ /* check if the font is valid */
			fnt_above = font->next->cmds[0]; /* size above the base line of text */
			fnt_below = font->next->cmds[1]; /* size below the base line of text */
			if((fnt_above + fnt_below) > 0){
				fnt_size = fnt_above + fnt_below;
			}
			if(fnt_above > 0) txt_size = 1/fnt_above;
		}
	}
	
	/*sweep the string, decoding utf8 */
	while (ofs = utf8_to_codepoint(txt + str_start, &code_p)){
		str_start += ofs;
		
		/* get final graphics of each code point*/
		curr_graph = shp_parse_cp(font, pool_idx, code_p, &width);
		width *= txt_size;
		if (curr_graph){
			graph_modify(curr_graph, ofs_x, 0.0, txt_size, txt_size, 0.0);
			/* store the graph in the return vector */
			if (list_push(list_ret, list_new((void *)curr_graph, pool_idx))) num_graph++;
		}
		/* update the ofset */
		ofs_x += width;
		
	}
	if (w != NULL) *w = ofs_x; /* return the text width*/
	return num_graph;
}