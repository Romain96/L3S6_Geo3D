#include "viewer.h"
#include <QGLViewer/camera.h>
#include <QGLViewer/vec.h>

#include <QKeyEvent>
#include <iomanip>
#include <QDebug>

Viewer::Viewer():
	QGLViewer(),
	m_render_mode(0),
	ROUGE(1,0,0),
	VERT(0,1,0),
	BLEU(0,0,1),
	JAUNE(1,1,0),
	CYAN(0,1,1),
	MAGENTA(1,0,1),
	BLANC(1,1,1),
	GRIS(0.5,0.5,0.5),
	NOIR(0,0,0),
	m_selected_quad(-1)
{}


void Viewer::init()
{
	makeCurrent();
	glewExperimental = GL_TRUE;
	int glewErr = glewInit();
	if( glewErr != GLEW_OK )
	{
		qDebug("Error %s", glewGetErrorString(glewErr) ) ;
	}
	std::cout << "GL VERSION = " << glGetString(GL_VERSION) << std::endl;

	// la couleur de fond
	glClearColor(0.2,0.2,0.2,0.0);

	// QGLViewer initialisation de la scene
	setSceneRadius(4.0);
	setSceneCenter(qglviewer::Vec(0.0,0.0,0.0));
	camera()->showEntireScene();
	setSceneRadius(20.0);

	// initialisation variables globales
	m_compteur = 0;

	m_prim.gl_init();

	m_mesh.gl_init();
}



void Viewer::draw_repere(const Mat4& global)
{
    auto fleche = [&] (Mat4 tr, Vec3 coul) -> void
    {
        m_prim.draw_cylinder(tr*translate(0,0,0.5)*scale(0.25,0.25,0.75), coul);
        m_prim.draw_cone(tr*translate(0,0,1)*scale(0.5,0.5,0.5), coul);
    };

    m_prim.draw_sphere(global*scale(0.5,0.5,0.5), BLANC);
    fleche(global, BLEU);
    fleche(global*rotateY(90), ROUGE);
    fleche(global*rotateX(-90), VERT);
}


void Viewer::draw()
{
	makeCurrent();

	m_mesh.set_matrices(getCurrentModelViewMatrix(),getCurrentProjectionMatrix());
	m_prim.set_matrices(getCurrentModelViewMatrix(),getCurrentProjectionMatrix());

	m_mesh.draw(CYAN);

	draw_repere(m_selected_frame);
}


void Viewer::keyPressEvent(QKeyEvent *event)
{
	switch(event->key())
	{
		case Qt::Key_Escape:
			exit(EXIT_SUCCESS);
			break;

		case Qt::Key_C:
			// Attention ctrl c utilise pour screen-shot !
			if (!(event->modifiers() & Qt::ControlModifier))
				m_mesh.create_cube();
            break;

        // e extrusion
        // +/- decale
        // z/Z shrink
        // t/T tourne

        // Attention au cas m_selected_quad == -1

        // extrusion
        case Qt::Key_E:
            if (m_selected_quad < 0)
            {
                qDebug() << "extrusion du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                break;
            }
            else
            {
                qDebug() << "extrusion du quad : " << m_selected_quad;
                m_mesh.extrude_quad(m_selected_quad);
                break;
            }

        // décalage positif
        case Qt::Key_Plus:
            if (m_selected_quad < 0)
            {
                qDebug() << "décalage positif du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                break;
            }
            else
            {
                qDebug() << "décalage positif du quad : " << m_selected_quad;
                m_mesh.decale_quad(m_selected_quad, 0.5f);
                break;
            }

        // décalage négatif
        case Qt::Key_Minus:
            if (m_selected_quad < 0)
            {
                qDebug() << "décalage négatif du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                break;
            }
            else
            {
                qDebug() << "décalage négatif du quad : " << m_selected_quad;
                m_mesh.decale_quad(m_selected_quad, -0.5f);
                break;
            }

        // homothétie (shrink)
        case Qt::Key_S:
            // agrandissement si (Shift + S)
            if (event->modifiers() & Qt::ShiftModifier)
            {
                if (m_selected_quad < 0)
                {
                    qDebug() << "homothétie (agrandissement) du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                    break;
                }
                else
                {
                    qDebug() << "homotétie du quad (agrandissement): " << m_selected_quad;
                    m_mesh.shrink_quad(m_selected_quad, 1.25f);
                    break;
                }
            }
            else
            {
                if (m_selected_quad < 0)
                {
                    qDebug() << "homothétie (réduction) du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                    break;
                }
                else
                {
                    qDebug() << "homotétie du quad (réduction): " << m_selected_quad;
                    m_mesh.shrink_quad(m_selected_quad, 0.75f);
                    break;
                }
            }

        // rotation
        case Qt::Key_T:
            // rotation dans le sens trigo si (T)
            if (event->modifiers() & Qt::ShiftModifier)
            {
                if (m_selected_quad < 0)
                {
                    qDebug() << "rotation (sen trigo) du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                    break;
                }
                else
                {
                    qDebug() << "rotation (sene trigo) du quad : " << m_selected_quad;
                    m_mesh.tourne_quad(m_selected_quad, 10.0f*M_PI/180.0f);
                    break;
                }
            }
            // rotation dans le sens anti horaire sinon (T + Shift)
            else
            {
                if (m_selected_quad < 0)
                {
                    qDebug() << "rotation (sen aiti horaire) du quad impossible (m_selected_quad = " << m_selected_quad << ")";
                    break;
                }
                else
                {
                    qDebug() << "rotation (sens anti horaire) du quad : " << m_selected_quad;
                    m_mesh.tourne_quad(m_selected_quad, 350.0f*M_PI/180.0f);
                    break;
                }
            }

		default:
			break;
	}

	// retrace la fenetre
	updateGL();
	QGLViewer::keyPressEvent(event);
}


void Viewer::mousePressEvent(QMouseEvent* event)
{
	// recupere le rayon de la souris dans la scene (P,Dir)
	qglviewer::Vec Pq = camera()->unprojectedCoordinatesOf(qglviewer::Vec(event->x(), event->y(), -1.0));
	qglviewer::Vec Qq = camera()->unprojectedCoordinatesOf(qglviewer::Vec(event->x(), event->y(), 1.0));
	Vec3 P(Pq[0],Pq[1],Pq[2]);
	Vec3 Dir(Qq[0]-Pq[0],Qq[1]-Pq[1],Qq[2]-Pq[2]);

	if (event->modifiers() & Qt::ShiftModifier)
	{
		m_selected_quad = m_mesh.intersected_visible(P,Dir);
        qDebug() << "nouveau quad sélectionné : " << m_selected_quad;
		if (m_selected_quad>=0)
		{
			m_selected_frame = m_mesh.local_frame(m_selected_quad);
		}
	}

	updateGL();
	QGLViewer::mousePressEvent(event);
}




Mat4 Viewer::getCurrentModelViewMatrix() const
{
	GLdouble gl_mvm[16];
	camera()->getModelViewMatrix(gl_mvm);
	Mat4 mvm;
	for(unsigned int i = 0; i < 4; ++i)
	{
		for(unsigned int j = 0; j < 4; ++j)
			mvm[i][j] = float(gl_mvm[i*4+j]);
	}
	return mvm;
}


Mat4 Viewer::getCurrentProjectionMatrix() const
{
	GLdouble gl_pm[16];
	camera()->getProjectionMatrix(gl_pm);
	Mat4 pm;
	for(unsigned int i = 0; i < 4; ++i)
	{
		for(unsigned int j = 0; j < 4; ++j)
			pm[i][j] = float(gl_pm[i*4+j]);
	}
	return pm;
}
