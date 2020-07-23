struct st
{
int default_mode;
int default_width;
int default_height;
int fmt_width;
int fmt_height;
int numctrls;
int numfmts;
int def_clk_freq;
unsigned int frame_length;
unsigned int coarse_time;
unsigned int gain;
};

extern struct st* camera_parameters( void );

extern const struct seq_operations ov5693_proc_op;

