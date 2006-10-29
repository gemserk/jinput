package net.java.games.input;

import java.io.IOException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import net.java.games.input.Component.Identifier;

public class WinTabComponent extends AbstractComponent {

	public final static int XAxis = 1;
	public final static int YAxis = 2;
	public final static int ZAxis = 3;
	public final static int NPressureAxis = 4;
	public final static int TPressureAxis = 5;
	public final static int OrientationAxis = 6;
	public final static int RotationAxis = 7;

	private int parentDevice;
	private int min;
	private int max;
	private WinTabContext context;
	protected float lastKnownValue;
	private boolean analog;

	protected WinTabComponent(WinTabContext context, int parentDevice, String name, Identifier id, int min, int max) {
		super(name, id);
		this.parentDevice = parentDevice;
		this.min = min;
		this.max = max;
		this.context = context;
		analog = true;
	}

	protected WinTabComponent(WinTabContext context, int parentDevice, String name, Identifier id) {
		super(name, id);
		this.parentDevice = parentDevice;
		this.min = 0;
		this.max = 1;
		this.context = context;
		analog = false;
	}

	protected float poll() throws IOException {
		return lastKnownValue;
	}
	
	public boolean isAnalog() {
		return analog;
	}

	public boolean isRelative() {
		// All axis are absolute
		return false;
	}

	public static List createComponents(WinTabContext context, int parentDevice, int axisId, int[] axisRanges) {
		List components = new ArrayList();
		Identifier id;
		switch(axisId) {
		case XAxis:
			id = Identifier.Axis.X;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			break;
		case YAxis:
			id = Identifier.Axis.Y;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			break;
		case ZAxis:
			id = Identifier.Axis.Z;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			break;
		case NPressureAxis:
			id = Identifier.Axis.X_FORCE;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			break;
		case TPressureAxis:
			id = Identifier.Axis.Y_FORCE;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			break;
		case OrientationAxis:
			id = Identifier.Axis.RX;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			id = Identifier.Axis.RY;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[2], axisRanges[3]));
			id = Identifier.Axis.RZ;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[4], axisRanges[5]));
			break;
		case RotationAxis:
			id = Identifier.Axis.RX;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[0], axisRanges[1]));
			id = Identifier.Axis.RY;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[2], axisRanges[3]));
			id = Identifier.Axis.RZ;
			components.add(new WinTabComponent(context, parentDevice, id.getName(), id, axisRanges[4], axisRanges[5]));
			break;
		}
		
		return components;
	}

	public static Collection createButtons(WinTabContext context, int deviceIndex, int numberOfButtons) {
		List buttons = new ArrayList();
		Identifier id;
		
		for(int i=0;i<numberOfButtons;i++) {
			try {
				Class buttonIdClass = Identifier.Button.class;
				Field idField = buttonIdClass.getField("_" + i);
				id = (Identifier)idField.get(null);
				buttons.add(new WinTabButtonComponent(context, deviceIndex, id.getName(), id, i));
			} catch (SecurityException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (NoSuchFieldException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		return buttons;
	}

	public Event processPacket(WinTabPacket packet) {
		// Set this to be the old value incase we don't change it		
		float newValue=lastKnownValue;
		
		if(getIdentifier()==Identifier.Axis.X) {
			newValue = normalise(packet.PK_X);
		}
		if(getIdentifier()==Identifier.Axis.Y) {
			newValue = normalise(packet.PK_Y);
		}
		if(getIdentifier()==Identifier.Axis.Z) {
			newValue = normalise(packet.PK_Z);
		}
		if(getIdentifier()==Identifier.Axis.X_FORCE) {
			newValue = normalise(packet.PK_NORMAL_PRESSURE);
		}
		if(getIdentifier()==Identifier.Axis.Y_FORCE) {
			newValue = normalise(packet.PK_TANGENT_PRESSURE);
		}
		if(getIdentifier()==Identifier.Axis.RX) {
			newValue = normalise(packet.PK_ORIENTATION_ALT);
		}
		if(getIdentifier()==Identifier.Axis.RY) {
			newValue = normalise(packet.PK_ORIENTATION_AZ);
		}
		if(getIdentifier()==Identifier.Axis.RZ) {
			newValue = normalise(packet.PK_ORIENTATION_TWIST);
		}
		if(newValue!=getPollData()) {
			lastKnownValue = newValue;
			
			//Generate an event
			Event newEvent = new Event();
			newEvent.set(this, newValue, packet.PK_TIME*1000);
			return newEvent;
		}
		
		return null;
	}
	
	private float normalise(float value) {
		if(max == min) return value;
		float bottom = max - min;
		return (value - min)/bottom;
	}

	public static Collection createCursors(WinTabContext context, int deviceIndex, String[] cursorNames) {
		Identifier id;
		List cursors = new ArrayList();
		
		for(int i=0;i<cursorNames.length;i++) {
			if(cursorNames[i].matches("Puck")) {
				id = Identifier.Button.TOOL_FINGER;
			} else if(cursorNames[i].matches("Eraser.*")) {
				id = Identifier.Button.TOOL_RUBBER;
			} else {
				id = Identifier.Button.TOOL_PEN;
			}
			cursors.add(new WinTabCursorComponent(context, deviceIndex, id.getName(), id, i));
		}
		
		return cursors;
	}
}
